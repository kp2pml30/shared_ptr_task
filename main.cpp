#include <gtest/gtest.h>
#include "shared_ptr.h"
#include "test_object.h"

template <typename T>
struct custom_deleter
{
    explicit custom_deleter(bool* deleted)
        : deleted(deleted)
    {}

    void operator()(T* object)
    {
        *deleted = true;
        delete object;
    }

private:
    bool* deleted;
};

struct base
{};

struct derived : base
{
    explicit derived(bool* deleted)
        : deleted(deleted)
    {}

    ~derived()
    {
        *deleted = true;
    }

private:
    bool* deleted;
};

TEST(shared_ptr_testing, default_ctor)
{
    shared_ptr<test_object> p;
    EXPECT_EQ(nullptr, p.get());
    EXPECT_FALSE(static_cast<bool>(p));
}

TEST(shared_ptr_testing, ptr_ctor)
{
    test_object::no_new_instances_guard g;
    test_object* p = new test_object(42);    
    shared_ptr<test_object> q(p);
    EXPECT_TRUE(static_cast<bool>(q));
    EXPECT_EQ(p, q.get());
    EXPECT_EQ(42, *q);
}

TEST(shared_ptr_testing, ptr_ctor_nullptr)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(nullptr);
    EXPECT_FALSE(static_cast<bool>(p));
    EXPECT_EQ(0, p.use_count());
}

TEST(shared_ptr_testing, ptr_ctor_non_empty_nullptr)
{
    shared_ptr<test_object> p(static_cast<test_object*>(nullptr));
    EXPECT_FALSE(static_cast<bool>(p));
    EXPECT_EQ(1, p.use_count());
}

TEST(shared_ptr_testing, ptr_ctor_inheritance)
{
    bool deleted = false;
    {
        shared_ptr<base> p(new derived(&deleted));
    }
    EXPECT_TRUE(deleted);
}

TEST(shared_ptr_testing, copy_ctor)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    EXPECT_EQ(1, p.use_count());
    shared_ptr<test_object> q = p;
    EXPECT_TRUE(static_cast<bool>(p));
    EXPECT_TRUE(static_cast<bool>(q));
    EXPECT_TRUE(p == q);
    EXPECT_EQ(42, *p);
    EXPECT_EQ(42, *q);
    EXPECT_EQ(2, q.use_count());
}

TEST(shared_ptr_testing, copy_ctor_nullptr)
{
    shared_ptr<test_object> p;
    shared_ptr<test_object> q = p;
    EXPECT_FALSE(static_cast<bool>(p));
    EXPECT_FALSE(static_cast<bool>(q));
}

TEST(shared_ptr_testing, const_dereferencing)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> const p(new test_object(42));
    EXPECT_EQ(42, *p);
    EXPECT_EQ(42, p->operator int());
}

TEST(shared_ptr_testing, reset)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> q(new test_object(42));
    EXPECT_TRUE(static_cast<bool>(q));
    q.reset();
    EXPECT_FALSE(static_cast<bool>(q));
}

TEST(shared_ptr_testing, reset_nullptr)
{
    shared_ptr<test_object> q;
    EXPECT_FALSE(static_cast<bool>(q));
    q.reset();
    EXPECT_FALSE(static_cast<bool>(q));
}

TEST(shared_ptr_testing, reset_ptr)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> q(new test_object(42));
    EXPECT_TRUE(static_cast<bool>(q));
    q.reset(new test_object(43));
    EXPECT_EQ(43, *q);
}

TEST(shared_ptr_testing, reset_ptr_inheritance)
{
    bool deleted = false;
    {
        shared_ptr<base> p;
        p.reset(new derived(&deleted));
    }
    EXPECT_TRUE(deleted);
}

TEST(shared_ptr_testing, move_ctor)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    shared_ptr<test_object> q = std::move(p);
    EXPECT_FALSE(static_cast<bool>(p));
    EXPECT_TRUE(static_cast<bool>(q));
    EXPECT_EQ(42, *q);
}

TEST(shared_ptr_testing, move_ctor_nullptr)
{
    shared_ptr<test_object> p;
    shared_ptr<test_object> q = std::move(p);
    EXPECT_FALSE(static_cast<bool>(p));
    EXPECT_FALSE(static_cast<bool>(q));
}

TEST(shared_ptr_testing, assignment_operator)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    shared_ptr<test_object> q(new test_object(43));
    p = q;
    EXPECT_EQ(43, *p);
    EXPECT_TRUE(p == q);
}

TEST(shared_ptr_testing, assignment_operator_from_nullptr)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    shared_ptr<test_object> q;
    p = q;
    EXPECT_FALSE(static_cast<bool>(p));
}

TEST(shared_ptr_testing, assignment_operator_to_nullptr)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p;
    shared_ptr<test_object> q(new test_object(43));
    p = q;
    EXPECT_EQ(43, *p);
    EXPECT_TRUE(p == q);
}

TEST(shared_ptr_testing, assignment_operator_nullptr)
{
    shared_ptr<test_object> p;
    shared_ptr<test_object> q;
    p = q;
    EXPECT_FALSE(static_cast<bool>(p));
}

TEST(shared_ptr_testing, assignment_operator_const)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    shared_ptr<test_object> const q(new test_object(43));
    p = q;
    EXPECT_EQ(43, *p);
    EXPECT_TRUE(p == q);
}

TEST(shared_ptr_testing, assignment_operator_self)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    p = p;
    EXPECT_EQ(42, *p);
}

TEST(shared_ptr_testing, assignment_operator_self_nullptr)
{
    shared_ptr<test_object> p;
    p = p;
    EXPECT_FALSE(static_cast<bool>(p));
}

TEST(shared_ptr_testing, move_assignment_operator)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    shared_ptr<test_object> q(new test_object(43));
    p = std::move(q);
    EXPECT_EQ(43, *p);
    EXPECT_FALSE(static_cast<bool>(q));
}

TEST(shared_ptr_testing, move_assignment_operator_from_nullptr)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    shared_ptr<test_object> q;
    p = std::move(q);
    EXPECT_FALSE(static_cast<bool>(p));
    EXPECT_FALSE(static_cast<bool>(q));
}

TEST(shared_ptr_testing, move_assignment_operator_to_nullptr)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p;
    shared_ptr<test_object> q(new test_object(43));
    p = std::move(q);
    EXPECT_EQ(43, *p);
    EXPECT_FALSE(static_cast<bool>(q));
}

TEST(shared_ptr_testing, move_assignment_operator_nullptr)
{
    shared_ptr<test_object> p;
    shared_ptr<test_object> q;
    p = std::move(q);
    EXPECT_FALSE(static_cast<bool>(p));
    EXPECT_FALSE(static_cast<bool>(q));
}

TEST(shared_ptr_testing, move_assignment_operator_self)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    p = std::move(p);
    EXPECT_EQ(42, *p);
}

TEST(shared_ptr_testing, move_assignment_operator_self_nullptr)
{
    shared_ptr<test_object> p;
    p = std::move(p);
    EXPECT_FALSE(static_cast<bool>(p));
}

TEST(shared_ptr_testing, weak_ptr_lock)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    weak_ptr<test_object> q = p;
    shared_ptr<test_object> r = q.lock();
    EXPECT_TRUE(r == p);
    EXPECT_EQ(42, *r);
}

TEST(shared_ptr_testing, weak_ptr_lock_nullptr)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    weak_ptr<test_object> q = p;
    p.reset();
    g.expect_no_instances();
    shared_ptr<test_object> r = q.lock();
    EXPECT_FALSE(static_cast<bool>(r));
}

TEST(shared_ptr_testing, weak_ptr_lock_nullptr_2)
{
    weak_ptr<test_object> q;
    EXPECT_FALSE(static_cast<bool>(q.lock()));
}

TEST(shared_ptr_testing, weak_ptr_copy_ctor)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    weak_ptr<test_object> q = p;
    weak_ptr<test_object> r = q;
}

TEST(shared_ptr_testing, weak_ptr_copy_ctor_nullptr)
{
    weak_ptr<test_object> p;
    weak_ptr<test_object> q = p;
}

TEST(shared_ptr_testing, weak_ptr_move_ctor)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    weak_ptr<test_object> q = p;
    weak_ptr<test_object> r = std::move(q);
    shared_ptr<test_object> s = r.lock();
    EXPECT_TRUE(p == s);
}

TEST(shared_ptr_testing, weak_ptr_move_ctor_nullptr)
{
    weak_ptr<test_object> p;
    weak_ptr<test_object> q = p;
}

TEST(shared_ptr_testing, weak_ptr_assignment_operator)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p1(new test_object(42));
    weak_ptr<test_object> q1 = p1;
    shared_ptr<test_object> p2(new test_object(43));
    weak_ptr<test_object> q2 = p2;

    q1 = q2;

    EXPECT_TRUE(q1.lock() == p2);
    EXPECT_TRUE(q2.lock() == p2);
}

TEST(shared_ptr_testing, weak_ptr_assignment_operator_from_nullptr)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p1(new test_object(42));
    weak_ptr<test_object> q1 = p1;
    weak_ptr<test_object> q2;

    q1 = q2;

    EXPECT_FALSE(static_cast<bool>(q1.lock()));
    EXPECT_FALSE(static_cast<bool>(q2.lock()));
}

TEST(shared_ptr_testing, weak_ptr_assignment_operator_to_nullptr)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    weak_ptr<test_object> q1;
    weak_ptr<test_object> q2 = p;

    q1 = q2;

    EXPECT_TRUE(q1.lock() == p);
    EXPECT_TRUE(q2.lock() == p);
}

TEST(shared_ptr_testing, weak_ptr_assignment_operator_nullptr)
{
    weak_ptr<test_object> q1;
    weak_ptr<test_object> q2;

    q1 = q2;

    EXPECT_FALSE(static_cast<bool>(q1.lock()));
    EXPECT_FALSE(static_cast<bool>(q2.lock()));
}

TEST(shared_ptr_testing, weak_ptr_assignment_operator_self)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    weak_ptr<test_object> q = p;

    q = q;

    EXPECT_TRUE(q.lock() == p);
}

TEST(shared_ptr_testing, weak_ptr_assignment_operator_self_nullptr)
{
    test_object::no_new_instances_guard g;
    weak_ptr<test_object> q;

    q = q;

    EXPECT_FALSE(static_cast<bool>(q.lock()));
}

TEST(shared_ptr_testing, weak_ptr_move_assignment_operator)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p1(new test_object(42));
    weak_ptr<test_object> q1 = p1;
    shared_ptr<test_object> p2(new test_object(43));
    weak_ptr<test_object> q2 = p2;

    q1 = std::move(q2);

    EXPECT_TRUE(q1.lock() == p2);
    EXPECT_FALSE(static_cast<bool>(q2.lock()));
}

TEST(shared_ptr_testing, weak_ptr_move_assignment_operator_from_nullptr)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p1(new test_object(42));
    weak_ptr<test_object> q1 = p1;
    weak_ptr<test_object> q2;

    q1 = std::move(q2);

    EXPECT_FALSE(static_cast<bool>(q1.lock()));
    EXPECT_FALSE(static_cast<bool>(q2.lock()));
}

TEST(shared_ptr_testing, weak_ptr_move_assignment_operator_to_nullptr)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    weak_ptr<test_object> q1;
    weak_ptr<test_object> q2 = p;

    q1 = std::move(q2);

    EXPECT_TRUE(q1.lock() == p);
    EXPECT_FALSE(static_cast<bool>(q2.lock()));
}

TEST(shared_ptr_testing, weak_ptr_move_assignment_operator_nullptr)
{
    weak_ptr<test_object> q1;
    weak_ptr<test_object> q2;

    q1 = std::move(q2);

    EXPECT_FALSE(static_cast<bool>(q1.lock()));
    EXPECT_FALSE(static_cast<bool>(q2.lock()));
}

TEST(shared_ptr_testing, weak_ptr_move_assignment_operator_self)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    weak_ptr<test_object> q = p;

    q = std::move(q);

    EXPECT_TRUE(q.lock() == p);
}

TEST(shared_ptr_testing, weak_ptr_move_assignment_operator_self_nullptr)
{
    test_object::no_new_instances_guard g;
    weak_ptr<test_object> q;

    q = std::move(q);

    EXPECT_FALSE(static_cast<bool>(q.lock()));
}

TEST(shared_ptr_testing, custom_deleter)
{
    test_object::no_new_instances_guard g;
    bool deleted = false;
    {
        shared_ptr<test_object> p(new test_object(42), custom_deleter<test_object>(&deleted));
    }
    EXPECT_TRUE(deleted);
}

TEST(shared_ptr_testing, custom_deleter_reset)
{
    test_object::no_new_instances_guard g;
    bool deleted;
    {
        shared_ptr<test_object> p;
        p.reset(new test_object(42), custom_deleter<test_object>(&deleted));
    }
    EXPECT_TRUE(deleted);
}

TEST(shared_ptr_testing, make_shared)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p = make_shared<test_object>(42);
    EXPECT_EQ(42, *p);
}

TEST(shared_ptr_testing, make_shared_weak_ptr)
{
    test_object::no_new_instances_guard g;
    weak_ptr<test_object> p;
    {
        shared_ptr<test_object> q = make_shared<test_object>(42);
        p = q;
    }
    g.expect_no_instances();
}

TEST(shared_ptr_testing, aliasing_ctor)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    int x;
    shared_ptr<int> q(p, &x);
    EXPECT_EQ(2, p.use_count());
    EXPECT_EQ(2, q.use_count());
}

TEST(shared_ptr_testing, aliasing_ctor_nullptr_non_empty)
{
    test_object::no_new_instances_guard g;
    shared_ptr<test_object> p(new test_object(42));
    shared_ptr<int> q(p, nullptr);
    EXPECT_EQ(2, p.use_count());
    EXPECT_EQ(2, q.use_count());
    EXPECT_TRUE(q.get() == nullptr);
}

TEST(shared_ptr_testing, comparison_with_nullptr)
{
    shared_ptr<test_object> p;
    EXPECT_TRUE(p == nullptr);
    EXPECT_FALSE(p != nullptr);
    EXPECT_TRUE(nullptr == p);
    EXPECT_FALSE(nullptr != p);
}

TEST(shared_ptr_testing, conversions_const)
{
    shared_ptr<test_object> p(new test_object(42));
    shared_ptr<test_object const> q = p;
    EXPECT_EQ(42, *q);
}

TEST(shared_ptr_testing, conversions_inheritance)
{
    struct base
    {};
    struct derived : base
    {};

    shared_ptr<derived> d(new derived());
    shared_ptr<base> b = d;
    EXPECT_EQ(d.get(), b.get());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
