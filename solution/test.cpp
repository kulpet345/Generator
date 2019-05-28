#include "generator.hpp"

#include <twist/test_framework/test_framework.hpp>

#include <memory>
#include <list>

struct TreeNode;

using TreeNodePtr = std::shared_ptr<TreeNode>;

struct TreeNode {
  int val_;
  TreeNodePtr left_;
  TreeNodePtr right_;

  TreeNode(int val = 0, TreeNodePtr left = nullptr, TreeNodePtr right = nullptr)
    : val_(val), left_(std::move(left)), right_(std::move(right)){
  }

  static TreeNodePtr Create(TreeNodePtr left, TreeNodePtr right, int val) {
    return std::make_shared<TreeNode>(val, std::move(left), std::move(right));
  }

  static TreeNodePtr CreateLeaf(int val) {
    return std::make_shared<TreeNode>(val);
  }
};


TEST_SUITE(Generator) {
  void TreeWalk(TreeNodePtr node) {
    if (node->left_) {
      TreeWalk(node->left_);
    }
    gen::Yield(node->val_);
    if (node->right_) {
      TreeWalk(node->right_);
    }
  }

  SIMPLE_TEST(CheckYield) {
    gen::Generator<int> gen([]() {
        gen::Generator<int>::Yield(5); 
    });

    auto val = gen.Resume();
    ASSERT_TRUE(val.has_value());
    ASSERT_EQ(val.value(), 5);
    val = gen.Resume();
    ASSERT_FALSE(val.has_value());
  }

  SIMPLE_TEST(TreeWalk) {
    TreeNodePtr root = TreeNode::Create(
      TreeNode::CreateLeaf(8),
      TreeNode::Create(
        TreeNode::Create(
          TreeNode::CreateLeaf(2),
          TreeNode::CreateLeaf(7),
          9
        ),
        TreeNode::CreateLeaf(5),
        1
      ),
      6
    );

    gen::Generator<int> walker([&root]() {
      TreeWalk(root);
    });

    size_t node_count = 0;
    std::vector<int> result;

    for (int x : walker) {
      result.push_back(x);
      ++node_count;
    }

    ASSERT_EQ(node_count, 7);
    std::vector<int> expected = {8, 6, 2, 9, 7, 1, 5};
    ASSERT_EQ(result, expected);
  }

  void ListGeneration(const std::list<int>& list) {
    for (int value : list) {
      gen::Yield(value);
    }
  }

  SIMPLE_TEST(ListGenerator) {
    std::list<int> list = {3, 7, 5, 98, 2, 4};
    
    gen::Generator<int> generator([&list]() {
      ListGeneration(list);
    });

    std::vector<int> result;

    while (true) {
      std::optional<int> value = generator.Resume();
      if (value.has_value()) {
        result.push_back(value.value());
      } else {
        break;
      }
    }

    std::vector<int> expected = {3, 7, 5, 98, 2, 4};
    ASSERT_EQ(result, expected);
  }
}

RUN_ALL_TESTS()
