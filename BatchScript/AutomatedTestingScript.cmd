@echo off
setlocal EnableDelayedExpansion

:: Define parameters headless timedilation numseeds input_strategy cellsize 
set parameters1=true 1.0 0 vanilla 1
::set parameters2=true 1.0 0 vanilla 1
::set parameters3=true 5.0 100 vanilla 1

:: Store in "array"
set total[0]=parameters1
::set total[1]=parameters2
::set total[2]=parameters3

:: Get total length
call :arraylength iterations
set /A iterations=%iterations%-1
echo Array length is %iterations%

:: Path to Unreal Engine executable and project (Edit as needed)
set UnrealExe="C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe"
set ProjectPath="C:\Users\Jason\Documents\Unreal Projects\TestGround\TestGround.uproject"

set VizualizationPath="C:\Users\Jason\Documents\SChool\research\go-explore\Go-ExploreAnalysis.ipynb"

:: Outer loop
for /l %%i in (0,1,%iterations%) do (
    :: Get the parameter set name
    set paramName=!total[%%i]!

    :: Extract the values from the current parameters using CALL to evaluate the variable
    call set paramValues=%%!paramName!%%

    :: print param values for debug
    echo Processing parameter set !paramName!
    echo Param values: !paramValues!

    :: Extract the values from the current parameters
    for /f "tokens=1,2,3,4,5" %%a in ("!paramValues!") do (
        :: Assign each token to a specific argument with delayed expansion
        set "HeadlessMode=%%a"
        set "TimeDilation=%%b"
        set "NumSeeds=%%c"
	set "Input_Strategy=%%d"
	set "CellSize=%%e"

        :: Debugging: Print out the values 
        echo HeadlessMode=!HeadlessMode!, TimeDilation=!TimeDilation!, NumSeeds=!NumSeeds!, Input_Strategy=!Input_Strategy!, CellSize = !CellSize!

        :: Start Unreal Engine instance with extracted parameters
        start "" %UnrealExe% %ProjectPath% -game -log -headless=!HeadlessMode! -timedilation=!TimeDilation! -numseed=!NumSeeds! -input_strategy=!Input_Strategy! -cellsize=!CellSize!
    )
)

:: Wait for all Unreal Engine instances to finish
:waitForUnreal
tasklist /FI "IMAGENAME eq UnrealEditor.exe" 2>NUL | find /I /N "UnrealEditor.exe">NUL
if "%ERRORLEVEL%"=="0" (
    echo Waiting for Unreal Engine instances to finish...
    timeout /t 5 >nul
    goto waitForUnreal
)

::start "" %VizualizationPath%

exit /b 0

:arraylength
SET /a x=0
:SymLoop

if defined total[%x%] (
    set /a x+=1
    goto SymLoop
)

:: Return the value of x by setting it to the passed argument
set "%~1=%x%"
exit /b 0




::start python script for vizualization after
