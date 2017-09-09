@echo off

for /f "usebackq tokens=1* delims=: " %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Workload.NativeDesktop`) do (
   if /i "%%i"=="installationPath" <nul set /p ".=%%j" > vc_path.txt
)