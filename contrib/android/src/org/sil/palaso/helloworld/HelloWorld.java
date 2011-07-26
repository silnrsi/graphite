/*  GRAPHITE2 LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.

    Alternatively, you may use this library under the terms of the Mozilla
    Public License (http://mozilla.org/MPL) or under the GNU General Public
    License, as published by the Free Sofware Foundation; either version
    2 of the license or (at your option) any later version.
*/

package org.sil.palaso.helloworld;

import android.app.Activity;
import android.graphics.Typeface;
import android.os.Bundle;
import android.webkit.WebView;
import android.widget.TextView;
import org.sil.palaso.Graphite;

public class HelloWorld extends Activity {
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	Typeface tfp = (Typeface)Graphite.addFontResource(getAssets(), "Padauk.ttf", "padauk", 0);
    	Typeface tfa = (Typeface)Graphite.addFontResource(getAssets(), "Scheherazadegr.ttf", "Scheh", 1);
    	Graphite.loadGraphite();
    	
    	TextView tv;
    	WebView wv;
//    	String s = "မဂင်္ဂလာ|မဘ္ဘာ၊ ဤကဲ့|သို့|ရာ|ဇ|ဝင်|တင်|မည့် ကြေ|ညာ|ချက်|ကို ပြု|လုပ်|ပြီး|နောက် ဤညီ|လာ|ခံ|အ|စည်း|အ|ဝေး|ကြီး|က ကမ္ဘာ့|ကု|လ|သ|မဂ္ဂ|အ|ဖွဲ့|ဝင် နိုင်|ငံ အား|လုံး|အား ထို|ကြေ|ညာ|စာ|တမ်း|ကြီး၏ စာ|သား|ကို|အ|များ|ပြည်|သူ|တို့ ကြား|သိ|စေ|ရန် ကြေ|ညာ|ပါ|မည့် အ|ကြောင်း|ကို|လည်း|ကောင်း၊ ထို့|ပြင်|နိုင်|ငံ|များ၊ သို့|တည်း|မ|ဟုတ် နယ်|မြေ|များ၏ နိုင်|ငံ|ရေး အ|ဆင့်|အ|တ|န်း|ကို လိုက်၍ ခွဲ|ခြား|ခြင်း မ|ပြု|ဘဲ|အ|ဓိ|က|အား|ဖြင့် စာ|သင်|ကျောင်း|များ|နှင့် အ|ခြား|ပ|ညာ|ရေး အ|ဖွဲ့|အ|စည်း|များ|တွင် ထို|ကြေ|ညာ|စာ|တမ်း|ကြီး|ကို ဖြန့်|ချိ ဝေ|ငှ စေ|ရန်၊ မြင်|သာ|အောင် ပြ|သ|ထား|စေ|ရန်၊|ဖတ်|ကြား|စေ|ရန်|နှင့် အ|ဓိပ္ပါယ်|ရှင်း|လင်း ဖော်|ပြ|စေ|ရန် ဆောင်|ရွက်|ပါ|မည့် အ|ကြောင်း|ဖြင့် လည်း|ကောင်း ဆင့်|ဆို လိုက်|သည်။".replace("|", "\u200B");
    	String s = "لمّا كان الاعتراف بالكرامة المتأصلة في جميع أعضاء الأسرة البشرية وبحقوقهم المتساوية الثابتة هو أساس الحرية والعدل \u06F1\u06F2\u06F3 والسلام في العالم.";
    	String w = "\uFEFF<html><body style=\"font-family: Scheh\">Test: " + s + "</body></html>";
//    	Typeface tf = Typeface.createFromAsset(getAssets(), "Padauk.ttf");
    	
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        tv = (TextView) findViewById(R.id.tv);
        tv.setText(s);
        tv.setTypeface(tfa, 0);
        tv.setTextSize(tv.getTextSize() * 2);
        wv = (WebView) findViewById(R.id.wv);
        wv.loadData(w, "text/html", "UTF-8");
    }
}
