# UESnippets
Unreal Engine Code Example &amp; Snippets

```bash
cd $GAMEPROJECT/Source
git clone https://github.com/david-pp/UESnippets.git 
```



## Windows Platform

Environment Values:

```bash
UnrealEnginePath=E:\UnrealEngine-4.27.2
```

Build :

```bash
# BuildProgram.bat
@echo off
set PROGRAME=%1
%UnrealEnginePath%\Engine\Binaries\DotNET\UnrealBuildTool.exe %PROGRAME% Win64 Development -Project="%cd%\HelloUE.uproject"
```

```bash
BuildProgram.bat YourSnippet
```
