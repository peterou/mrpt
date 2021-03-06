/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          http://www.mrpt.org/                          |
   |                                                                        |
   | Copyright (c) 2005-2017, Individual contributors, see AUTHORS file     |
   | See: http://www.mrpt.org/Authors - All rights reserved.                |
   | Released under BSD License. See details in http://www.mrpt.org/License |
   +------------------------------------------------------------------------+ */

#include <mrpt/typemeta/TEnumType.h>
#include <gtest/gtest.h>

// Example declaration of "enum class"
enum class TestColors
{
	Black = 0,
	Gray = 7,
	White = 15
};

MRPT_ENUM_TYPE_BEGIN(TestColors)
MRPT_FILL_ENUM_MEMBER(TestColors, Black);
MRPT_FILL_ENUM_MEMBER(TestColors, Gray);
MRPT_FILL_ENUM_MEMBER(TestColors, White);
MRPT_ENUM_TYPE_END()

// Example declaration of plain enum
enum Directions
{
	North,
	East,
	South,
	West
};
// Example declaration of "enum class"
MRPT_ENUM_TYPE_BEGIN(Directions)
MRPT_FILL_ENUM(North);
MRPT_FILL_ENUM(East);
MRPT_FILL_ENUM(South);
MRPT_FILL_ENUM(West);
MRPT_ENUM_TYPE_END()

TEST(TEnumType, str2value)
{
	using mrpt::typemeta::TEnumType;

	EXPECT_EQ(TEnumType<TestColors>::name2value("White"), TestColors::White);
	EXPECT_EQ(TEnumType<TestColors>::name2value("Black"), TestColors::Black);
	EXPECT_EQ(TEnumType<TestColors>::name2value("Gray"), TestColors::Gray);

	EXPECT_EQ(TEnumType<Directions>::name2value("East"), East);

	try
	{
		TEnumType<TestColors>::name2value("Violet");
		EXPECT_FALSE(true) << "Expected exception but it didn't happen!";
	}
	catch (std::exception&)
	{
		// All good
	}
}
