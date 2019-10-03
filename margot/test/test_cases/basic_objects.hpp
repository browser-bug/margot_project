// MyTestSuite1.h
#include <cxxtest/TestSuite.h>

#include <margot/basic_information_block.hpp>

class BasicInformationBlock : public CxxTest::TestSuite
{
  public:


    void test_data_point(void)
    {
      // declaring some data number
      margot::Data<float> number1(3.4f);
      margot::Data<unsigned int> number2(3);
      margot::Data<float> number3(3);
      margot::Data<float> number4(3.4f);

      // test that they have a mean
      TS_ASSERT(margot::traits::has_mean<decltype(number1)>::value);
      TS_ASSERT(margot::traits::has_mean<decltype(number2)>::value);
      TS_ASSERT(margot::traits::has_mean<decltype(number3)>::value);
      TS_ASSERT(margot::traits::has_mean<decltype(number4)>::value);

      // test that they don't have a standard deviation
      TS_ASSERT(!margot::traits::has_standard_deviation<decltype(number1)>::value);
      TS_ASSERT(!margot::traits::has_standard_deviation<decltype(number2)>::value);
      TS_ASSERT(!margot::traits::has_standard_deviation<decltype(number3)>::value);
      TS_ASSERT(!margot::traits::has_standard_deviation<decltype(number4)>::value);

      // test the equal operator
      TS_ASSERT(number1 == number4);
      TS_ASSERT(!(number1 == number3));
      TS_ASSERT(number1 != number3);
      TS_ASSERT(!(number1 != number4))

    }


    void test_data_distribution(void)
    {
      // declaring some data number
      margot::Distribution<float> number1(3.4f);
      margot::Distribution<unsigned int> number2(3, 4);
      margot::Distribution<float> number3(3, 1);
      margot::Distribution<float> number4(3.4f, 2);

      // test that they have a mean
      TS_ASSERT(margot::traits::has_mean<decltype(number1)>::value);
      TS_ASSERT(margot::traits::has_mean<decltype(number2)>::value);
      TS_ASSERT(margot::traits::has_mean<decltype(number3)>::value);
      TS_ASSERT(margot::traits::has_mean<decltype(number4)>::value);

      // test that they don't have a standard deviation
      TS_ASSERT(margot::traits::has_standard_deviation<decltype(number1)>::value);
      TS_ASSERT(margot::traits::has_standard_deviation<decltype(number2)>::value);
      TS_ASSERT(margot::traits::has_standard_deviation<decltype(number3)>::value);
      TS_ASSERT(margot::traits::has_standard_deviation<decltype(number4)>::value);

      // test the equal operator
      TS_ASSERT(number1 == number4);
      TS_ASSERT(!(number1 == number3));
      TS_ASSERT(number1 != number3);
      TS_ASSERT(!(number1 != number4))

    }

};
