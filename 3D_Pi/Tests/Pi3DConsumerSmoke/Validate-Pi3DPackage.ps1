param(
	[Parameter(Mandatory = $false)]
	[string]$PackageRoot
)

$ErrorActionPreference = "Stop"
if ([string]::IsNullOrWhiteSpace($PackageRoot)) {
	$PackageRoot = Join-Path $PSScriptRoot "..\..\Addons\Pi3D"
}
$packagePath = [System.IO.Path]::GetFullPath($PackageRoot)
$packagePrefix = $packagePath.TrimEnd([System.IO.Path]::DirectorySeparatorChar) + [System.IO.Path]::DirectorySeparatorChar
$errors = [System.Collections.Generic.List[string]]::new()

# 必須ファイルの存在を検証する。
function Test-RequiredFile {
	param(
		[Parameter(Mandatory = $true)]
		[string]$RelativePath
	)

	$fullPath = Join-Path $packagePath $RelativePath
	if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
		$errors.Add("Missing required file: $RelativePath")
	}
}

if (-not (Test-Path -LiteralPath $packagePath -PathType Container)) {
	throw "Pi3D package directory was not found: $packagePath"
}

$requiredSourceFiles = @(
	"Pi3D.hpp",
	"Pi3DConfig.hpp",
	"PostEffects\Effects.cpp",
	"PostEffects\Effects_BayerDither.cpp",
	"PostEffects\Effects_Bloom.cpp",
	"PostEffects\Effects_CRT.cpp",
	"PostEffects\Effects_DoF.cpp",
	"PostEffects\Effects_ExtractBright.cpp",
	"PostEffects\Effects_FilmGrain.cpp",
	"PostEffects\Effects_FXAA.cpp",
	"PostEffects\Effects_Glitch.cpp",
	"PostEffects\Effects_Grayscale.cpp",
	"PostEffects\Effects_Invert.cpp",
	"PostEffects\Effects_Kuwahara.cpp",
	"PostEffects\Effects_Mosaic.cpp",
	"PostEffects\Effects_None.cpp",
	"PostEffects\Effects_Outline.cpp",
	"PostEffects\Effects_PixelArt.cpp",
	"PostEffects\Effects_Posterize.cpp",
	"PostEffects\Effects_RGBShift.cpp",
	"PostEffects\Effects_Swirl.cpp",
	"PostEffects\Effects_TonemapACES.cpp",
	"PostEffects\Effects_Toon.cpp",
	"PostEffects\Effects_Vignette.cpp",
	"PostEffects\Effects_WarmGrade.cpp"
)

$shaderNames = @(
	"bayer_dither",
	"bloom_extract",
	"crt",
	"dof_combine",
	"extract_bright_linear",
	"film_grain",
	"fxaa",
	"glitch",
	"grayscale",
	"invert",
	"kuwahara",
	"mosaic",
	"outline",
	"pixelart",
	"posterize",
	"rgb_shift",
	"scene_fog",
	"scene_underwater_distort",
	"scene_underwater_fog",
	"swirl",
	"tonemap_aces",
	"toon",
	"vignette",
	"warm_grade"
)

$requiredResourceFiles = [System.Collections.Generic.List[string]]::new()
foreach ($shaderName in $shaderNames) {
	$requiredResourceFiles.Add("Resources\shader\hlsl\$shaderName.hlsl")
	$requiredResourceFiles.Add("Resources\shader\glsl\$shaderName.frag")
}
$requiredResourceFiles.Add("Resources\shader\hlsl\dof_depth.hlsl")
$requiredResourceFiles.Add("Resources\shader\glsl\dof_depth.frag")
$requiredResourceFiles.Add("Resources\shader\glsl\dof_depth.vert")

foreach ($textureName in @("ame.png", "effectEditor.png", "hatena.png", "kaihei.png", "ryou.png", "zimenTexture.png")) {
	$requiredResourceFiles.Add("Resources\texture\$textureName")
}
$requiredResourceFiles.Add("Resources\toml\effect_presets.toml")
$requiredResourceFiles.Add("Resources\toml\lighting_presets.toml")

foreach ($relativePath in $requiredSourceFiles) {
	Test-RequiredFile -RelativePath $relativePath
}
foreach ($relativePath in $requiredResourceFiles) {
	Test-RequiredFile -RelativePath $relativePath
}

$sourceFiles = Get-ChildItem -LiteralPath $packagePath -Recurse -File | Where-Object {
	$_.Extension -in @(".hpp", ".ipp", ".cpp")
}
$includePattern = '^\s*#\s*include\s*"([^"]+)"'
foreach ($sourceFile in $sourceFiles) {
	foreach ($line in Get-Content -LiteralPath $sourceFile.FullName) {
		if ($line -notmatch $includePattern) {
			continue
		}

		$includePath = [System.IO.Path]::GetFullPath((Join-Path $sourceFile.DirectoryName $Matches[1]))
		if (-not $includePath.StartsWith($packagePrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
			$relativeSource = $sourceFile.FullName.Substring($packagePrefix.Length)
			$errors.Add("Package boundary violation: $relativeSource includes $($Matches[1])")
			continue
		}

		if (-not (Test-Path -LiteralPath $includePath -PathType Leaf)) {
			$relativeSource = $sourceFile.FullName.Substring($packagePrefix.Length)
			$errors.Add("Missing local include: $relativeSource includes $($Matches[1])")
		}
	}
}

if ($errors.Count -ne 0) {
	$errors | ForEach-Object { Write-Error $_ }
	throw "Pi3D package validation failed with $($errors.Count) error(s)."
}

Write-Host "Pi3D package validation passed. Sources=$($sourceFiles.Count), required files=$($requiredSourceFiles.Count + $requiredResourceFiles.Count)"
