// For lightweight reusable structs

#pragma once

typedef unsigned int uint32_t;

namespace nx
{
	template<typename T>
	struct Size
	{
	public:
		T Width, Height;

	public:
		// Custom C-style conversion
		operator Size<uint32_t>() const
		{ return Size<uint32_t> {(uint32_t)Width, (uint32_t)Height}; }
		operator Size<int>() const
		{ return Size<int> {(int)Width, (int)Height}; }
	};
}

namespace nx
{
	struct QueueFamilyIndices
	{
	public:
		int Graphics = UNAVAILABLE;
		int Present = UNAVAILABLE;

		static constexpr int UNAVAILABLE = -1;
		static constexpr int MIN_INDEX = 0;

	public:
		bool IsComplete()
		{ return Graphics >= MIN_INDEX && Present >= MIN_INDEX; }
	};
}