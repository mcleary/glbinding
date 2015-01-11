#include <gmock/gmock.h>

#include <thread>
#include <array>
#include <list>
#include <iostream>

#include <RingBuffer.h>

using namespace glbinding;

class RingBuffer_test : public testing::Test
{
public:
};

TEST_F(RingBuffer_test, SimpleTest)
{
    RingBuffer<int, 10> buffer;

    int result;
    EXPECT_EQ(false, buffer.pull(result));

    for (int i = 1; i <= 10; i++)
    {
        EXPECT_EQ(true, buffer.push(i));
    }
    EXPECT_EQ(10, buffer.size());
    EXPECT_EQ(false, buffer.push(11));

    for (int i = 1; i <= 5; i++)
    {
        EXPECT_EQ(true, buffer.pull(result));
        EXPECT_EQ(i, result);
    }

    EXPECT_EQ(5, buffer.size());

    for (int i = 1; i <= 3; i++)
    {
        EXPECT_EQ(true, buffer.push(i));
    }

    EXPECT_EQ(8, buffer.size());
}

TEST_F(RingBuffer_test, StringTest)
{
    RingBuffer<std::string, 10> buffer;

    std::string result;
    EXPECT_EQ(false, buffer.pull(result));

    for (int i = 1; i <= 10; i++)
    {
        EXPECT_EQ(true, buffer.push("Hello world!"));
    }
    EXPECT_EQ(10, buffer.size());
    EXPECT_EQ(false, buffer.push("Not working!"));

    for (int i = 1; i <= 5; i++)
    {
        EXPECT_EQ(true, buffer.pull(result));
        EXPECT_EQ("Hello world!", result);
    }

    EXPECT_EQ(5, buffer.size());

    for (int i = 1; i <= 3; i++)
    {
        EXPECT_EQ(true, buffer.push("Hello world2!"));
    }

    EXPECT_EQ(8, buffer.size());
}

TEST_F(RingBuffer_test, MultiThreadedTest)
{

    RingBuffer<int, 3> buffer;
    std::array<int, 5> i1 = {{1, 2, 3, 4, 5}};
    std::list<int> out;
    std::thread t1([&]()
    {
        for(const int s: i1)
            while(!buffer.push(s));
    });

    std::thread t2([&]()
    {
        int result;
        for(const int s: i1)
        {
            while(!buffer.pull(result));
            EXPECT_EQ(s, result);
            out.push_front(result);
        }

    });

    t1.join();
    t2.join();
    EXPECT_EQ(5, out.size());
    EXPECT_EQ(0, buffer.size());
}

TEST_F(RingBuffer_test, MultiThreadedTest2)
{

    RingBuffer<int, 3> buffer;
    std::thread t1([&]()
    {
        for(int i = 0; i < 100000; i++)
            while(!buffer.push(i));
    });

    std::thread t2([&]()
    {
        int result;
        for(int j = 0; j < 100000; j++)
        {
            while(!buffer.pull(result));
            EXPECT_EQ(j, result);
        }

    });

    t1.join();
    t2.join();
    EXPECT_EQ(0, buffer.size());
}

TEST_F(RingBuffer_test, ConsumerTest1)
{
    RingBuffer<int, 10> buffer;

    unsigned int a = buffer.addTail();
    EXPECT_EQ(0, a);

    int result;

    for(int i = 0; i < 5; i++)
    {
        while(!buffer.push(i)){}
    }

    for(int j = 0; j < 3; j++)
    {
        buffer.pullTail(a, result);
        EXPECT_EQ(j, result);
    }

    unsigned int b = buffer.addTail();
    EXPECT_EQ(1, b);

    for(int j = 3; j < 5; j++)
    {
        buffer.pullTail(b, result);
        EXPECT_EQ(j, result);
    }

    for(int i = 5; i < 13; i++)
    {
        while(!buffer.push(i)){}
    }
    
    EXPECT_EQ(10, buffer.size());
    EXPECT_EQ(10, buffer.sizeTail(a));
    EXPECT_EQ(8, buffer.sizeTail(b));
    
    buffer.removeTail(a);
    EXPECT_EQ(8, buffer.size());

    unsigned int c = buffer.addTail();
    EXPECT_EQ(0, c);

    for(int j = 5; j < 9; j++)
    {
        buffer.pullTail(b, result);
        EXPECT_EQ(j, result);
    }

    buffer.removeTail(b);
    EXPECT_EQ(8, buffer.size());

    for(int j = 5; j < 13; j++)
    {
        buffer.pullTail(c, result);
        EXPECT_EQ(j, result);
    }

    EXPECT_EQ(0, buffer.size());
}

TEST_F(RingBuffer_test, ConsumerTest2)
{
    RingBuffer<int, 50> buffer;
    unsigned int a = buffer.addTail();
    unsigned int b = buffer.addTail();

    std::thread t1([&]()
    {
        for(int i = 0; i < 100000; i++)
            while(!buffer.push(i));
    });

    std::thread t2([&]()
    {
        int result;
        for(int j = 0; j < 100000; j++)
        {
            while(!buffer.pullTail(a, result));
            EXPECT_EQ(j, result);
        }

    });

    std::thread t3([&]()
    {
        int result;
        for(int j = 0; j < 100000; j++)
        {
            while(!buffer.pullTail(b, result));
            EXPECT_EQ(j, result);
        }

    });

    t1.join();
    t2.join();
    t3.join();

    buffer.removeTail(a);
    buffer.removeTail(b);

    EXPECT_EQ(0, buffer.size());
}

TEST_F(RingBuffer_test, PullBlockTest)
{
    RingBuffer<int, 10> buffer;

    unsigned int a = buffer.addTail();
    EXPECT_EQ(0, a);

    for(int i = 0; i < 5; i++)
    {
        while(!buffer.push(i)){}
    }

    EXPECT_EQ(5, buffer.size());

    std::vector<int> result = buffer.pullTail(a, 4);
    EXPECT_EQ(4, result.size());

    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(i, result[i]);
    }

    EXPECT_EQ(1, buffer.size());

    for(int i = 5; i < 13; i++)
    {
        while(!buffer.push(i)){}
    }

    result = buffer.pullTail(a, 9);
    EXPECT_EQ(9, result.size());

    for (int i = 4; i < 13; i++)
    {
        EXPECT_EQ(i, result[i-4]);
    }

    EXPECT_EQ(0, buffer.size());
}

// TEST_F(RingBuffer_test, MultiThreadedTest2)
// {

//     RingBuffer<int, 7> buffer;
//     std::array<int, 10> i1 = {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}};
//     std::array<int, 10> i2 = {{10, 11, 12, 13, 14, 15, 16, 17, 18, 19}};
//     std::list<int> out;
//     std::thread t1([&]()
//     {
//         for(const int s: i1)
//             while(!buffer.push(s));
//     });

//     std::thread t2([&]()
//     {
//         for(const int s: i2)
//             while(!buffer.push(s));
//     });

//     std::thread t3([&]()
//     {
//         int result;
//         // std::cout << "End: " << i1.size() + i2.size() << std::endl;
//         for(int i = 0; i < i1.size() + i2.size(); i++)
//         {
//             while(!buffer.pull(result));
//             out.push_front(result);
//             std::cout << "i = " << result << std::endl;
//         }

//     });

//     t1.join();
//     t2.join();
//     t3.join();

//     EXPECT_EQ(i1.size() + i2.size(), out.size());
//     EXPECT_EQ(0, buffer.size());

//     out.sort();
//     for(const int s: i1)
//     {
//         EXPECT_EQ(s, out.front());
//         out.pop_front();
//     }
//     for(const int s: i2)
//     {
//         EXPECT_EQ(s, out.front());
//         out.pop_front();
//     }
// }



