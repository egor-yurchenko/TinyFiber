
#include "gtest/gtest.h"



//#include "../TinyFiber/scheduler.hpp"
//#include "../TinyFiber/scheduler.cpp"
//
//#include "../TinyFiber/context.hpp"
//#include "../TinyFiber/context.cpp"
//
//#include "../TinyFiber/mmap_allocation.cpp"
//#include "../TinyFiber/panic.cpp"
//#include "../TinyFiber/stack.cpp"
//#include "../TinyFiber/string_builder.hpp"
//#include "../TinyFiber/memspan.hpp"
//#include "../TinyFiber/assert.hpp"

#include "scheduler.hpp"

#include "context.hpp"
#include "string_builder.hpp"
#include "memspan.hpp"
#include "assert.hpp"

//#include "../TinyFiber/Coroutine.h"
#include "Coroutine.h"


//extern "C" void tinyfiber::SwitchContext(tinyfiber::ExecutionContext * from, tinyfiber::ExecutionContext * to);


TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}

TEST(ThreadPool, HelloWorld) {
    tinyfiber::ThreadPool tp{ 1 };
    tp.Submit([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Hello, World!" << std::endl;
        });
    tp.Join();
}

TEST(ThreadPool, WeNeedToGoDeeper) {
    tinyfiber::ThreadPool tp{ 1 };
    auto deeper = []() {
        tinyfiber::ThreadPool::Current()->Submit(
            []() {
                std::cout << "We need to go deeper..." << std::endl;
            });
        };
    tp.Submit(deeper);
    tp.Join();
}

TEST(ThreadPool, SubmitAferJoin) {
    tinyfiber::ThreadPool tp{ 3 };

    auto submit = [&tp]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));

        // After join
        tp.Submit([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Submitted after Join" << std::endl;
            });
        };

    tp.Submit(submit);

    tp.Join();
}

TEST(ThreadPool, Current) {
    tinyfiber::ThreadPool tp{ 4 };
    ASSERT_EQ(tinyfiber::ThreadPool::Current(), nullptr);

    auto task = [&tp]() {
        ASSERT_EQ(tinyfiber::ThreadPool::Current(), &tp);
        };

    tp.Submit(task);
    tp.Join();
}

//--------------------------------------------

struct TreeNode;

using tinyfiber::Coroutine;
using TreeNodePtr = std::shared_ptr<TreeNode>;

struct TreeNode {
    TreeNodePtr left_;
    TreeNodePtr right_;

    TreeNode(TreeNodePtr left = nullptr, TreeNodePtr right = nullptr)
        : left_(std::move(left)), right_(std::move(right)) {
    }

    static TreeNodePtr Create(TreeNodePtr left, TreeNodePtr right) {
        return std::make_shared<TreeNode>(std::move(left), std::move(right));
    }

    static TreeNodePtr CreateLeaf() {
        return std::make_shared<TreeNode>();
    }
};


TEST(Coroutine, JustWorks) {
    auto foo_routine = [&]() {
        Coroutine::Suspend();
        };

    tinyfiber::Coroutine foo(foo_routine);

    EXPECT_FALSE(foo.isCompleted());
    foo.Resume();
    foo.Resume();
    EXPECT_TRUE(foo.isCompleted());

    ASSERT_THROW(foo.Resume(), tinyfiber::CoroutineCompleted);
}

TEST(Coroutine, Interleaving) {
    int step = 0;

    auto finn_routine = [&]() {
        ASSERT_EQ(step, 0);
        step = 1;
        Coroutine::Suspend();
        ASSERT_EQ(step, 2);
        step = 3;
        };

    auto jake_routine = [&]() {
        ASSERT_EQ(step, 1);
        step = 2;
        Coroutine::Suspend();
        ASSERT_EQ(step, 3);
        step = 4;
        };

    tinyfiber::Coroutine finn(finn_routine);
    tinyfiber::Coroutine jake(jake_routine);

    finn.Resume();
    jake.Resume();

    ASSERT_EQ(step, 2);

    finn.Resume();
    jake.Resume();

    ASSERT_TRUE(finn.isCompleted());
    ASSERT_TRUE(jake.isCompleted());

    ASSERT_EQ(step, 4);
}

void TreeWalk(TreeNodePtr node) {
    if (node->left_) {
        TreeWalk(node->left_);
    }
    Coroutine::Suspend();
    if (node->right_) {
        TreeWalk(node->right_);
    }
}

TEST(Coroutine, TreeWalk) {
    TreeNodePtr root = TreeNode::Create(
        TreeNode::CreateLeaf(),
        TreeNode::Create(
            TreeNode::Create(
                TreeNode::CreateLeaf(),
                TreeNode::CreateLeaf()
            ),
            TreeNode::CreateLeaf()
        )
    );

    tinyfiber::Coroutine walker([&root]() {
        TreeWalk(root);
        });

    size_t node_count = 0;

    while (true) {
        walker.Resume();
        if (walker.isCompleted()) {
            break;
        }
        ++node_count;
    }

    ASSERT_EQ(node_count, 7);
}

TEST(Coroutine, Pipeline) {
    const size_t kSteps = 123;

    size_t inner_step_count = 0;

    auto inner_routine = [&]() {
        for (size_t i = 0; i < kSteps; ++i) {
            ++inner_step_count;
            Coroutine::Suspend();
        }
        };

    auto outer_routine = [&]() {
        tinyfiber::Coroutine inner(inner_routine);
        while (!inner.isCompleted()) {
            inner.Resume();
            Coroutine::Suspend();
        }
        };

    tinyfiber::Coroutine outer(outer_routine);
    while (!outer.isCompleted()) {
        outer.Resume();
    }

    ASSERT_EQ(inner_step_count, kSteps);
}

TEST(Coroutine, NotInCoroutine) {
    ASSERT_THROW(Coroutine::Suspend(), tinyfiber::NotInCoroutine);
}

//TEST(Coroutine, Exception) {
//    auto foo_routine = [&]() {
//        tinyfiber::Suspend();
//        throw std::runtime_error("Test exception");
//        };
//
//    tinyfiber::Coroutine foo(foo_routine);
//
//    ASSERT_FALSE(foo.isCompleted());
//    foo.Resume();
//    ASSERT_THROW(foo.Resume(), std::runtime_error);
//    ASSERT_TRUE(foo.isCompleted());
//}

// -------------------------