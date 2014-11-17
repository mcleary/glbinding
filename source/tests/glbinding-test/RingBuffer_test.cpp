#include <gmock/gmock.h>

#include <thread>

#include <glbinding/RingBuffer.hpp>

using namespace glbinding;

class RingBuffer_test : public testing::Test
{
public:
};

namespace 
{
    void error(int /*errnum*/, const char * /*errmsg*/)
    {
        FAIL();
    }
}

TEST_F(RingBuffer_test, Test)
{
    RingBuffer<int, 10> buffer;

    int result;
    EXPECT_EQ(false, buffer.pull(&result));

    for (int i = 1; i <= 10; i++)
    {
        EXPECT_EQ(true, buffer.push(i));
    }
    EXPECT_EQ(10, buffer.size());
    EXPECT_EQ(false, buffer.push(11));

    for (int i = 1; i <= 5; i++)
    {
        EXPECT_EQ(true, buffer.pull(&result));
        EXPECT_EQ(i, result);
    }
}
