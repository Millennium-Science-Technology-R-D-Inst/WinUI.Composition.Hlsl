module;
#include <cmath>
export module WinUI.Composition.Hlsl.Validation;
export namespace hlsl::validation
{
	inline bool IsFiniteNonNegative(double value) noexcept
	{
		return std::isfinite(value) && value >= 0;
	}
}
