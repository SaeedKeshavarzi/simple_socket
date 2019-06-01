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

IF EXIST ".\x64" (
    rmdir ".\x64" /s /q
)

IF EXIST ".\Debug" (
    rmdir ".\Debug" /s /q
)

IF EXIST ".\Release" (
    rmdir ".\Release" /s /q
)

del *.pro.user

del *.vcxproj.user

del *.db
