IF EXIST ".\.vs" (
    rmdir ".\.vs" /s /q
)

IF EXIST ".\bin" (
    rmdir ".\bin" /s /q
)

IF EXIST ".\obj" (
    rmdir ".\obj" /s /q
)

IF EXIST ".\ipch" (
    rmdir ".\ipch" /s /q
)

del *.db
