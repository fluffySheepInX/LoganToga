BirdModelAsset LoadBirdModelAsset(FilePathView path, const double displayHeight)
{
	const ImportedStaticMesh staticMesh = LoadStaticGlbMesh(path, displayHeight);
	ImportedAnimatedData animatedData = LoadAnimatedData(path);
	const Mat4x4 normalizationTransform = animatedData.vertices.isEmpty()
		? staticMesh.normalizationTransform
		: ComputeNormalizationTransform(animatedData, displayHeight);

	return{
		.mesh = staticMesh.mesh,
		.loaded = staticMesh.loaded,
       .staticVertices = std::move(staticMesh.vertices),
		.staticIndices = std::move(staticMesh.indices),
		.vertices = std::move(animatedData.vertices),
		.indices = std::move(animatedData.indices),
		.vertexNodeIndices = std::move(animatedData.vertexNodeIndices),
		.normalizationTransform = normalizationTransform,
		.nodes = std::move(animatedData.nodes),
		.bones = std::move(animatedData.bones),
		.animations = std::move(animatedData.animations),
		.subMeshes = std::move(animatedData.subMeshes),
	};
}
