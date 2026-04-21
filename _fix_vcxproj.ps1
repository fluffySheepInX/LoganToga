$p = '3D_Pi\3D_Pi.vcxproj'
$full = (Resolve-Path $p).Path
$c = [System.IO.File]::ReadAllText($full)
$needle = '    <FxCompile Include="App\example\shader\hlsl\warm_grade.hlsl" />' + "`r`n"
$c = $c.Replace($needle, '')
[System.IO.File]::WriteAllText($full, $c)
Write-Host 'done'
