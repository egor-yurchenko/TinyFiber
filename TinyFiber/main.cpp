#include <iostream>

#include "Coroutine.h"
#include "scheduler.hpp"
#include "fiber.h"
#include <cassert>

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


void JustWorks() {
    auto foo_routine = [&]() {
        Coroutine::Suspend();
        };

    tinyfiber::Coroutine foo(foo_routine);

    assert(foo.isCompleted() == false);
    foo.Resume();
    foo.Resume();
    assert(foo.isCompleted() == true);

    try {
        foo.Resume();
    }
    catch (tinyfiber::CoroutineCompleted& e) {
        std::cout << e.what() << std::endl;
    }
    catch (...) {
        assert(0);
    }
}

void Interleaving() {
    int step = 0;

    auto finn_routine = [&]() {
        assert(step == 0);
        step = 1;
        Coroutine::Suspend();
        assert(step == 2);
        step = 3;
        };

    auto jake_routine = [&]() {
        assert(step == 1);
        step = 2;
        Coroutine::Suspend();
        assert(step == 3);
        step = 4;
        };

    tinyfiber::Coroutine finn(finn_routine);
    tinyfiber::Coroutine jake(jake_routine);

    finn.Resume();
    jake.Resume();

    assert(step == 2);

    finn.Resume();
    jake.Resume();

    assert(finn.isCompleted());
    assert(jake.isCompleted());

    assert(step == 4);
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

void Coroutine_TreeWalk() {
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

    assert(node_count == 7);
}

void Pipeline() {
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

    assert(inner_step_count == kSteps);
}

void NotInCoroutine () {
    try {
        Coroutine::Suspend();
    }
    catch (tinyfiber::NotInCoroutine& e) {
        std::cout << e.what() << std::endl;
    }
}

/////// TO DO //////////////
void Exception () {
    auto foo_routine = [&]() {
        //Coroutine::Suspend();
        throw std::runtime_error("Test exception");
    };

    tinyfiber::Coroutine foo(foo_routine);

    assert(foo.isCompleted() == false);
    //foo.Resume();
    try {
        foo.Resume();
    }
    catch (std::runtime_error& e) {
        std::cout << e.what() << std::endl;
    }
    assert(foo.isCompleted());
}



struct MyException {
};

void NestedException1() {
    auto bar_routine = [&]() {
        throw MyException();
        };

    auto foo_routine = [&]() {
        tinyfiber::Coroutine bar(bar_routine);

        try {
            bar.Resume();
        }
        catch (MyException& e) {
            (void)e;
            std::cout << "MyException" << std::endl;
        }

        //ASSERT_THROW(bar.Resume(), MyException);
        };

    tinyfiber::Coroutine foo(foo_routine);
    foo.Resume();
}

void NestedException2() {
    auto bar_routine = [&]() {
        throw MyException();
        };

    auto foo_routine = [&]() {
        tinyfiber::Coroutine bar(bar_routine);
        bar.Resume();
        };

    tinyfiber::Coroutine foo(foo_routine);

    try {
        foo.Resume();
    }
    catch (MyException& e) {
        (void)e;
        std::cout << "MyException" << std::endl;
    }

    //ASSERT_THROW(foo.Resume(), MyException);
}

// -----------------------------------

void Leak() {
    auto shared_ptr = std::make_shared<int>(42);
    std::weak_ptr<int> weak_ptr = shared_ptr;

    {
        auto routine = [ptr = std::move(shared_ptr)]() {};
        tinyfiber::Coroutine co(routine);
        co.Resume();
    }

    assert(!weak_ptr.lock());
}

// ----------------------------------------------

static void RunScheduler(tinyfiber::FiberRoutine init, size_t threads) {
    tinyfiber::ThreadPool thread_pool{ threads };
    tinyfiber::Spawn(init, thread_pool);
    thread_pool.Join();
}

using tinyfiber::ThreadPool;
using tinyfiber::Spawn;
using tinyfiber::Yieldd;

void InsideThreadPool() {
    ThreadPool tp{ 3 };
    std::atomic<bool> done{ false };

    auto tester = [&]() {
        assert(ThreadPool::Current() == &tp);
        done.store(true);
        };

    Spawn(tester, tp);
    tp.Join();

    assert(done.load() == true);
}

void ChildInsideThreadPool() {
    ThreadPool tp{ 3 };
    std::atomic<size_t> done{ 0 };

    auto tester = [&tp, &done]() {
        assert(ThreadPool::Current() == &tp);

        auto child = [&tp, &done]() {
            assert(ThreadPool::Current() == &tp);
            done.fetch_add(1);
            };
        Spawn(child);

        done.fetch_add(1);
        };

    Spawn(tester, tp);
    tp.Join();

    assert(done.load() == 2);
}

void RunInParallel() {
    ThreadPool tp{ 4 };
    std::atomic<size_t> completed{ 0 };

    auto sleeper = [&completed]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        completed.fetch_add(1);
        };

    auto start = std::chrono::system_clock::now();

    Spawn(sleeper, tp);
    Spawn(sleeper, tp);
    Spawn(sleeper, tp);
    tp.Join();

    assert(completed.load() == 3);
    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    assert(elapsed/1000 < 3.5);
}

void YieldTest() {
    std::atomic<int> value{ 0 };

    auto check_value = [&value]() {
        const int kLimit = 10;

        assert(value.load() < kLimit);
        assert(value.load() > -kLimit);
        };

    static const size_t kIterations = 12345;

    auto bull = [&]() {
        for (size_t i = 0; i < kIterations; ++i) {
            value.fetch_add(1);
            Yieldd();
            check_value();
        }
        };

    auto bear = [&]() {
        for (size_t i = 0; i < kIterations; ++i) {
            value.fetch_sub(1);
            Yieldd();
            check_value();
        }
        };

    auto starter = [&]() {
        Spawn(bull);
        Spawn(bear);
        };

    ThreadPool tp{ 1 };
    Spawn(starter, tp);
    tp.Join();
}

void Yield2Test(){
    ThreadPool tp{ 4 };

    static const size_t kYields = 123456;

    auto tester = []() {
        for (size_t i = 0; i < kYields; ++i) {
            Yield();
        }
        };

    Spawn(tester, tp);
    Spawn(tester, tp);

    tp.Join();
}


class ForkTester {
public:
    ForkTester(size_t threads)
        : pool_(threads) {
    }

    size_t Explode(size_t d) {
        Spawn(MakeForker(d), pool_);
        pool_.Join();
        return leafs_.load();
    }

private:
    tinyfiber::FiberRoutine MakeForker(size_t d) {
        return [this, d]() {
            if (d > 2) {
                Spawn(MakeForker(d - 2));
                Spawn(MakeForker(d - 1));
            }
            else {
                leafs_.fetch_add(1);
            }
            };
    }

private:
    ThreadPool pool_;
    std::atomic<size_t> leafs_{ 0 };
};

void Forks() {
    ForkTester tester{ 4 };
    assert(tester.Explode(21) == 10946);
}

void TwoPools1() {
    ThreadPool tp_1{ 4 };
    ThreadPool tp_2{ 4 };

    auto make_tester = [](ThreadPool& tp) {
        return [&tp]() {
            assert(ThreadPool::Current() == &tp);
            };
        };

    Spawn(make_tester(tp_1), tp_1);
    Spawn(make_tester(tp_2), tp_2);
}

void TwoPools() {
    ThreadPool tp_1{ 4 };
    ThreadPool tp_2{ 4 };

    auto make_tester = [](ThreadPool& tp) {
        return [&tp]() {
            static const size_t kIterations = 1024;

            for (size_t i = 0; i < kIterations; ++i) {
                assert(ThreadPool::Current() == &tp);

                Yield();

                Spawn([&tp]() {
                    assert(ThreadPool::Current() == &tp);
                    });
            }
            };
        };

    Spawn(make_tester(tp_1), tp_1);
    Spawn(make_tester(tp_2), tp_2);
}

struct RacyCounter {
public:
    void Increment() {
        value_.store(
            value_.load(std::memory_order_relaxed) + 1,
            std::memory_order_relaxed);
    }
    size_t Get() const {
        return value_.load(std::memory_order_relaxed);
    }
private:
    std::atomic<size_t> value_{ 0 };
};

void RacyCounterTest() {
    static const size_t kIncrements = 100'000;
    static const size_t kThreads = 4;
    static const size_t kFibers = 100;

    RacyCounter counter;

    auto routine = [&]() {
        for (size_t i = 0; i < kIncrements; ++i) {
            counter.Increment();
            if (i % 10 == 0) {
                tinyfiber::Yieldd();
            }
        }
        };

    auto init = [&]() {
        for (size_t i = 0; i < kFibers; ++i) {
            tinyfiber::Spawn(routine);
        }
        };

    RunScheduler(init, kThreads);

    std::cout << "Thread count: " << kThreads << std::endl
        << "Fibers: " << kFibers << std::endl
        << "Increments per fiber: " << kIncrements << std::endl
        << "Racy counter value: " << counter.Get() << std::endl;

    assert(counter.Get() >= kIncrements);
    assert(counter.Get() < kIncrements * kFibers);
}

struct A {
    A(std::function<void()> f) : func_(std::move(f)), exception_(nullptr){};

    void run() {
        try {
            func_();
        }
        catch (...) {
            exception_ = std::current_exception();
        }
    }

    void getException() {
        if (exception_) {
            std::rethrow_exception(exception_);
        }
    }

    std::function<void()> func_;
    std::exception_ptr exception_;
};

int main() {
	
    JustWorks();
    Interleaving();
    Coroutine_TreeWalk();
    Pipeline();
    NotInCoroutine();


    // TODO
    //Exception(); 
    //NestedException1();
    //NestedException2();

    Leak();

    InsideThreadPool();
    ChildInsideThreadPool();
    RunInParallel();
    YieldTest();
    Yield2Test();

    Forks();
    TwoPools1();
    TwoPools();
    RacyCounterTest();

	return 0;
}