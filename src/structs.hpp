// For lightweight reusable structs

#pragma once

using uint32_t = unsigned int;

namespace ppr
{
	template<typename T>
	struct size
	{
	public:
		T width, height;

	public:
		// Custom C-style conversions
		operator size<uint32_t>() const
		{ return size<uint32_t> {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }

		operator size<int>() const
		{ return size<int> {static_cast<int>(width), static_cast<int>(height)}; }
	};
}

namespace ppr
{
	struct queue_families
	{
	public:
		int graphics = UNAVAILABLE;
		int present = UNAVAILABLE;

		static constexpr int UNAVAILABLE = -1;
		static constexpr int MIN_INDEX = 0;

	public:
		bool is_complete() const
		{ return graphics >= MIN_INDEX && present >= MIN_INDEX; }
	};
}