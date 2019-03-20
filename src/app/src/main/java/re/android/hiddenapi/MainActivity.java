package re.android.hiddenapi;
import android.view.View;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("hiddenapi-bypass");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        check(null);

        Button check_btn = (Button) findViewById(R.id.check_btn);
        check_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                check(view);
            }
        });
        Button disable_btn = (Button) findViewById(R.id.disable_btn);
        disable_btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                disable(view);
            }
        });


    }
    public void check(View view) {
        TextView namespace_txt = (TextView) findViewById(R.id.namespace_txt);
        String ns_txt = "Namespace: ";
        if (this.isProtectedNamespaceEnabled()) {
            ns_txt += "Activated";
        } else {
            ns_txt += "Deactivated";
        }
        namespace_txt.setText(ns_txt);

        TextView hidden_txt = (TextView) findViewById(R.id.hiddenapi_txt);
        String hd_txt = "Hidden API: ";

        boolean hactivated = true;
        try {
          hactivated = this.isHiddenApiEnabled();
        } catch (NoSuchMethodError err) {
          hactivated = true;
        }

        if (hactivated) {
            hd_txt += "Activated";
        } else {
            hd_txt += "Deactivated";
        }
        hidden_txt.setText(hd_txt);

    }

    public void disable(View view) {
        this.disableProtectedNamespace();
        this.disableHiddenApi();
    }

    public native boolean isHiddenApiEnabled();
    public native boolean isProtectedNamespaceEnabled();

    public native void disableProtectedNamespace();
    public native void disableHiddenApi();
}
