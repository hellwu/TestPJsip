package com.hellw.testpjsip;

/**
 * Created by hellw on 2018/9/6.
 */

public class Sip {
    static {
        System.loadLibrary("MySip");
    }
    public native void initSip();
    public native void regc();
    public native void destroySip();
    public native void test();
}
