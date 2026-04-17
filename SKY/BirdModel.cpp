# include "BirdModel.hpp"
# include "BirdModelLoader.hpp"

namespace
{
	struct NodeTransformParts
	{
		Float3 scale{ 1.0f, 1.0f, 1.0f };
		Quaternion rotation = Quaternion::Identity();
		Float3 translation{ 0.0f, 0.0f, 0.0f };
	};

	[[nodiscard]] NodeTransformParts DecomposeTransform(const Mat4x4& transform) noexcept
	{
		NodeTransformParts result;
		transform.decompose(result.scale, result.rotation, result.translation);
		return result;
	}

	[[nodiscard]] Mat4x4 ComposeTransform(const NodeTransformParts& transform) noexcept
	{
		return Mat4x4::Identity()
			.scaled(transform.scale)
			.rotated(transform.rotation)
			.translated(transform.translation);
	}

	[[nodiscard]] Float3 TransformNormalByPointPair(const Mat4x4& transform, const Float3& position, const Float3& normal) noexcept
	{
		const Float3 transformedPosition = transform.transformPoint(position);
		const Float3 transformedNormalEnd = transform.transformPoint(position + normal);
		const Float3 transformedNormal = (transformedNormalEnd - transformedPosition);

		if (transformedNormal.isZero())
		{
			return normal;
		}

		return transformedNormal.normalized();
	}

	[[nodiscard]] Array<Mat4x4> BuildWorldTransforms(const Array<UnitModelNode>& nodes, const Array<Mat4x4>& localTransforms)
	{
		Array<Mat4x4> worldTransforms(nodes.size(), Mat4x4::Identity());

		for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
		{
			const int32 parentIndex = nodes[nodeIndex].parentIndex;

			if (parentIndex < 0)
			{
				worldTransforms[nodeIndex] = localTransforms[nodeIndex];
			}
			else
			{
				worldTransforms[nodeIndex] = (localTransforms[nodeIndex] * worldTransforms[parentIndex]);
			}
		}

		return worldTransforms;
	}

   [[nodiscard]] Float3 SampleVectorKey(const Array<UnitModelAnimationKey<Float3>>& keys, const double time, const Float3& fallback)
	{
		if (keys.isEmpty())
		{
			return fallback;
		}

		if (keys.size() == 1)
		{
          return (time < keys.front().time)
				? fallback
				: keys.front().value;
		}

      if (time < keys.front().time)
		{
          return fallback;
		}

		for (size_t keyIndex = 1; keyIndex < keys.size(); ++keyIndex)
		{
			const auto& previous = keys[(keyIndex - 1)];
			const auto& next = keys[keyIndex];

			if (time <= next.time)
			{
				const double duration = Max((next.time - previous.time), 1e-8);
				const float t = static_cast<float>((time - previous.time) / duration);
				return (previous.value + ((next.value - previous.value) * t));
			}
		}

		return keys.back().value;
	}

   [[nodiscard]] Quaternion SampleQuaternionKey(const Array<UnitModelAnimationKey<Quaternion>>& keys, const double time, const Quaternion& fallback)
	{
		if (keys.isEmpty())
		{
			return fallback;
		}

		if (keys.size() == 1)
		{
          return (time < keys.front().time)
				? fallback
				: keys.front().value;
		}

      if (time < keys.front().time)
		{
          return fallback;
		}

		for (size_t keyIndex = 1; keyIndex < keys.size(); ++keyIndex)
		{
			const auto& previous = keys[(keyIndex - 1)];
			const auto& next = keys[keyIndex];

			if (time <= next.time)
			{
				const double duration = Max((next.time - previous.time), 1e-8);
				const float t = static_cast<float>((time - previous.time) / duration);
				return previous.value.slerp(next.value, t);
			}
		}

		return keys.back().value;
	}
}

# include "BirdModel.Core.inl"
# include "BirdModel.Animation.inl"
