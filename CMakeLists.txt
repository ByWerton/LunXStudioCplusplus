cmake_minimum_required(VERSION 3.10.2)
project("LunXStudioEngine")
KRİTİK NOT: Luau kütüphanesini burada harici olarak bağlamalıyız.
Gerçek projede Luau kaynak dosyaları (örneğin luau-vm/src/*.c) buraya eklenir.
Burası, Luau'nun LunXEngine motorunuza nasıl entegre olacağının mimarisidir.
Simülasyon için sadece motor çekirdeğini ekliyoruz.
C++ Kaynak Dosyaları
add_library(
native-lib
SHARED
native-lib.cpp
)
Gerekli kütüphaneleri bağla: OpenGL ES 3.0, Android ve Loglama
find_library(gles3-lib GLESv3)
target_link_libraries(
native-lib
android
${gles3-lib}
log
# Gerçek projede burada Luau kütüphanesini bağlardık:
# luau
)
