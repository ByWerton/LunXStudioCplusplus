package com.example.lunxstudio;

import androidx.appcompat.app.AppCompatActivity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

// Bu sınıf, C++ motoruyla konuşan aracıdır.
class MyGLRenderer implements GLSurfaceView.Renderer {
    // C++ tarafından çağrılacak native metodlar
    public native void onDrawFrame();
    public native void onSurfaceChanged(int width, int height);

    public void onSurfaceCreated(GL10 unused, EGLConfig config) {
        // C++ motorunun initializeEngine fonksiyonu, MainActivity.onCreate'te çağrılır.
    }

    public void onDrawFrame(GL10 unused) {
        onDrawFrame(); // C++ renderFrame() fonksiyonunu çağırır
    }

    public void onSurfaceChanged(GL10 unused, int width, int height) {
        onSurfaceChanged(width, height); 
    }
}

public class MainActivity extends AppCompatActivity {

    private GLSurfaceView gLView;

    // C++ kütüphanesini yükle
    static {
        // native-lib.cpp'den oluşturulan kütüphane
        System.loadLibrary("native-lib"); 
    }

    // C++'tan gelen başlangıç durumunu alacak fonksiyon
    public native String stringFromJNI();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // --- LunXStudio'nun 3D Ekranı ---
        gLView = new GLSurfaceView(this);
        gLView.setEGLContextClientVersion(3); // OpenGL ES 3.0 Kullan
        gLView.setRenderer(new MyGLRenderer()); 
        gLView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY); // Sürekli çizim

        setContentView(gLView);

        // Motoru başlat ve C++ durumunu kontrol et
        System.out.println("Motor Durumu: " + stringFromJNI());
    }
}
