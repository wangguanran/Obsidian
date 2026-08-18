/*
 *  Copyright 2023 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of NXP nor the names of its contributors may be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
package com.nxp.test.nfc;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.nfc.NfcAdapter;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.TextUtils;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Button;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatButton;
import androidx.appcompat.widget.Toolbar;
import androidx.core.content.ContextCompat;
import com.nxp.nfc.NxpNfcAdapter;
import com.nxp.nfc.TdaResult;
import com.nxp.nfc.CardTLVInfo;
import com.nxp.nfc.NfcTDAInfo;



public class NfcTdaTestActivity  extends AppCompatActivity  {
  private static final String TAG = "NfcTdaDemoApp";
  private TextView mResult;
  private NxpNfcAdapter mNxpNfcAdapter;
  private NfcAdapter mNfcAdapter;
  private Button openTda;
  private CheckBox mTdaSAM1, mTdaSAM2;
  private Button discoverTda;
  private TextView sam1;
  private TextView sam2;
  private TextView trnsCmd;
  private Button trnsSend;
  private TextView trnsRsp;
  private Button closeTda;

  private static final int TDA_SAM1_ID = 0x21;
  private static final int TDA_SAM2_ID = 0x22;
  static byte mTdaId = TDA_SAM1_ID;
  byte[] in_cmd_data = {  0x0B, 0x00, 0x14, 0x00, (byte)0xA4, 0x04,
                          0x00,0x0E, 0x31, 0x50, 0x41, 0x59, 0x2E,
                          0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46,
                          0x30, 0x31, 0x00};
  byte[] buffclean = {};
  byte mCID = 0x00;
  TdaResult openStatus = new TdaResult();
  TdaResult closeStatus = new TdaResult();

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    Toolbar toolbar = findViewById(R.id.toolbar);
    setSupportActionBar(toolbar);
    SpannableString nfcTitle = new SpannableString("Nfc");
    SpannableString tdaTitle = new SpannableString("TdaDemo");
    SpannableString testTitle = new SpannableString("App");

    nfcTitle.setSpan(new ForegroundColorSpan(ContextCompat.getColor(
                           getApplicationContext(), R.color.orange)),
                       0, nfcTitle.length(),
                       Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
    tdaTitle.setSpan(new ForegroundColorSpan(ContextCompat.getColor(
                          getApplicationContext(), R.color.blue)),
                      0, tdaTitle.length(),
                      Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
    testTitle.setSpan(new ForegroundColorSpan(ContextCompat.getColor(
                            getApplicationContext(), R.color.green)),
                        0, testTitle.length(),
                        Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

    getSupportActionBar().setTitle(
        TextUtils.concat(nfcTitle, tdaTitle, testTitle));
    ((TextView)toolbar.getChildAt(0)).setTextSize(40);
    mResult = findViewById(R.id.text_view);
    openTda = findViewById(R.id.openTda);
    mTdaSAM1 = (CheckBox)findViewById(R.id.tda_SAM1);
    mTdaSAM2 = (CheckBox)findViewById(R.id.tda_SAM2);
    sam1 = findViewById(R.id.sam1);
    sam2 = findViewById(R.id.sam2);
    discoverTda = findViewById(R.id.discoverTda);
    trnsSend = findViewById(R.id.trnsSend);
    trnsRsp = findViewById(R.id.trnsRsp);
    trnsCmd = findViewById(R.id.trnsCmd);
    closeTda = findViewById(R.id.closeTda);
    mNfcAdapter = NfcAdapter.getDefaultAdapter(this);
    mNxpNfcAdapter = NxpNfcAdapter.getNxpNfcAdapter(mNfcAdapter);
  }

  @Override
  protected void onStart() {
    super.onStart();
    openTda.setOnClickListener(onClickListener);
    mTdaSAM1.setOnCheckedChangeListener(onCheckedChangeListener);
    mTdaSAM2.setOnCheckedChangeListener(onCheckedChangeListener);
    discoverTda.setOnClickListener(onClickListener);
    trnsSend.setOnClickListener(onClickListener);
    closeTda.setOnClickListener(onClickListener);
    openTda.setEnabled(false);
    trnsSend.setEnabled(false);
    closeTda.setEnabled(true);
  }

  protected void onResume() {
    super.onResume();
  }

  private void displayTdaInfoOnUI(NfcTDAInfo mTdaInfo, int tda) {
      String formattedData = formatCTDataToString(mTdaInfo);
      if (tda == 0x00)
        sam1.setText(formattedData);
      else if (tda == 0x01)
        sam2.setText(formattedData);
  }

  private void displayTrnsApdu(byte[] rsp, boolean flag)
  {
    StringBuilder formattedData = new StringBuilder();
    if(flag == true) {
      formattedData.append("APDU CMD : ").append(byteArrayToHexString(rsp));
      trnsCmd.setText(formattedData);
    } else {
      formattedData.append("APDU RSP : ").append(byteArrayToHexString(rsp));
      trnsRsp.setText(formattedData);
    }
  }

  private String cidHexToString (byte byt) {
    StringBuilder sb = new StringBuilder();
    sb.append("CID: 0x");
    sb.append(String.format("%02X", byt & 0xFF));
    return sb.toString();
  }

  private String formatCTDataToString(NfcTDAInfo mTdaInfo) {
    StringBuilder formattedData = new StringBuilder();

    switch (mTdaInfo.id) {
      case 0x21 :
          formattedData.append("TDA ID : 0x").append(Integer.toHexString(mTdaInfo.id)).append("   --------------   SAM1 Card\n");
          break;
      case 0x22 :
          formattedData.append("TDA ID : 0x").append(Integer.toHexString(mTdaInfo.id)).append("   --------------   SAM2 Card\n");
          break;
      default :
          formattedData.append("TDA ID : 0x").append(Integer.toHexString(mTdaInfo.id)).append("   --------------   UNKNOWN Card\n");
          break;
    }

    switch (mTdaInfo.status) {
      case 0x00 :
          formattedData.append("Status : 0x").append(Integer.toHexString(mTdaInfo.status)).append("   -----------------   Connected and Enabled\n");
          break;
      case 0x01 :
          formattedData.append("Status : 0x").append(Integer.toHexString(mTdaInfo.status)).append("   -----------------   Connected and Disabled\n");
          break;
      case 0x02 :
          formattedData.append("Status : 0x").append(Integer.toHexString(mTdaInfo.status)).append("   -----------------   Removed Or Unresponsive\n");
          break;
      default :
          formattedData.append("Status : 0x").append(Integer.toHexString(mTdaInfo.status)).append("   -----------------   UNKNOWN state\n");
          break;
    }

    formattedData.append("Num. of Protocols : 0x").append(Integer.toHexString(mTdaInfo.numberOfProtocols)).append("\n");
    formattedData.append("Protocol Value : ").append(intArrayToHexString(mTdaInfo.protocols)).append("\n");
    formattedData.append("Num Of CardInfo : 0x").append(Integer.toHexString(mTdaInfo.numberOfCardInfo)).append("\n");
    if(mTdaInfo.numberOfCardInfo != 0) {
      for(int i = 0; i < mTdaInfo.numberOfCardInfo; i++)
      {
        formattedData.append("Type : 0x").append(Integer.toHexString(mTdaInfo.cardTLVInfo[i].type)).append("\n");
        formattedData.append("Length : 0x").append(Integer.toHexString(mTdaInfo.cardTLVInfo[i].length)).append("\n");
        formattedData.append("Value : ").append(byteArrayToHexString(mTdaInfo.cardTLVInfo[i].value));
      }
    } else{
      formattedData.append("CardInfo Not Available !!!!").append("\n");
    }
    return formattedData.toString();
  }


  private String intArrayToHexString(int[] byteArray) {
      StringBuilder sb = new StringBuilder();
      for (int i = 0; i < byteArray.length; i++) {
          sb.append("0x").append(Integer.toHexString(byteArray[i] & 0xFF));
          if (i < byteArray.length - 1) {
              sb.append(", ");
          }
      }
      return sb.toString();
  }

  private String byteArrayToHexString(byte[] byteArray) {
    StringBuilder sb = new StringBuilder();
    for (int i = 0; i < byteArray.length; i++) {
        sb.append(String.format("%02X", byteArray[i] & 0xFF));
    }
    return sb.toString();
  }

  private void onOpenTdaClick() {
    Log.d(TAG, "onOpenTdaClick: ");
    String cidString;

    mCID = mNxpNfcAdapter.openTDA(mTdaId,false,openStatus);
    in_cmd_data[0] = mCID;
    byte oSt = openStatus.getStatus();
    Log.d(TAG, "CID: " + mCID);
    Log.d(TAG, "OpenTda Status : " + oSt);
    cidString = cidHexToString(mCID);
    if(oSt == 0x00){
      displayTrnsApdu(in_cmd_data, true);
      trnsSend.setEnabled(true);
      closeTda.setEnabled(true);
      setEnableAllcheckBox(false);
      openTda.setEnabled(false);
      onDiscoverTdaClick();
      if(mCID == 0x0B){
        sam1.setBackgroundColor(getResources().getColor(R.color.green));
      } else if(mCID == 0x0C){
        sam2.setBackgroundColor(getResources().getColor(R.color.green));
      }
    } else {
      setCheckedAllcheckBox(false);
      setEnableAllcheckBox(true);
      openTda.setEnabled(false);
    }

    openTda.setBackgroundColor(getResources().getColor(R.color.green));
    openTda.setText(cidString);
    discoverTda.setBackgroundColor(getResources().getColor(R.color.orange));
    discoverTda.setText("DISCOVER TDA");
    closeTda.setBackgroundColor(getResources().getColor(R.color.orange));
    closeTda.setText("CLOSE TDA");
  }

  private void onTdaSam1Click(boolean isChecked) {
    mTdaId = TDA_SAM1_ID;
    if(isChecked == false) {
      openTda.setEnabled(false);
      mTdaSAM1.setEnabled(true);
      mTdaSAM2.setEnabled(true);
    } else {
      openTda.setEnabled(true);
      mTdaSAM2.setEnabled(false);
    }
    openTda.setBackgroundColor(getResources().getColor(R.color.orange));
    openTda.setText("Open TDA");
    Log.d(TAG, "onTdaSam1Click: " + isChecked);
  }

  private void onTdaSam2Click(boolean isChecked) {
    mTdaId = TDA_SAM2_ID;
    if(isChecked == false) {
      openTda.setEnabled(false);
      mTdaSAM1.setEnabled(true);
      mTdaSAM2.setEnabled(true);
    } else {
      openTda.setEnabled(true);
      mTdaSAM1.setEnabled(false);
    }
    openTda.setBackgroundColor(getResources().getColor(R.color.orange));
    openTda.setText("Open TDA");
    Log.d(TAG, "onTdaSam2Click: " + isChecked);
  }

  private void onSendTrnsClick() {
      trnsSend.setBackgroundColor(getResources().getColor(R.color.green));
      trnsSend.setText("APDU SENT");
      byte[] rsp_data;
      byte index = 0x00;
      TdaResult transceiveStatus = new TdaResult();
      rsp_data = mNxpNfcAdapter.transceive(in_cmd_data,transceiveStatus);
      byte trsSt = transceiveStatus.getStatus();
      Log.d(TAG, "trsSt: " + trsSt);
      if(trsSt == 0x00) {
        displayTrnsApdu(rsp_data, false);
        for(byte b : rsp_data)
        {
          String hexApdu = String.format("%02X", b);
          Log.d(TAG, " apdu_rsp[" + index + "] = 0x" + hexApdu);
          index++;
        }
      }
  }

  private void setEnableAllcheckBox(boolean flag)
  {
    mTdaSAM1.setEnabled(flag);
    mTdaSAM2.setEnabled(flag);
  }

  private void setCheckedAllcheckBox(boolean flag)
  {
    mTdaSAM1.setChecked(flag);
    mTdaSAM2.setChecked(flag);
  }

  private void onCloseTdaClick() {
          /* Close the TDA */
    StringBuilder formattedData = new StringBuilder();
    mNxpNfcAdapter.closeTDA(mTdaId,true,closeStatus);

    if(closeStatus.getStatus() == TdaResult.RESULT_SUCCESS) {
      /* Reset all button/checklist/click and read to take next input */
      openTda.setEnabled(false);
      setCheckedAllcheckBox(false);
      setEnableAllcheckBox(true);
      trnsSend.setEnabled(false);
      openTda.setBackgroundColor(getResources().getColor(R.color.orange));
      openTda.setText("Open TDA");
      closeTda.setBackgroundColor(getResources().getColor(R.color.green));
      closeTda.setText("Close TDA");
      discoverTda.setBackgroundColor(getResources().getColor(R.color.orange));
      discoverTda.setText("DISCOVER TDA");
      formattedData.append("APDU CMD : ").append(byteArrayToHexString(buffclean));
      trnsCmd.setText(formattedData);
      formattedData.setLength(0);
      formattedData.append("APDU RSP : ").append(byteArrayToHexString(buffclean));
      trnsRsp.setText(formattedData);
      trnsSend.setBackgroundColor(getResources().getColor(R.color.orange));
      trnsSend.setText("Send APDU");
      formattedData.setLength(0);
      formattedData.append("Discover TDA Info Not Available").append(byteArrayToHexString(buffclean));
      sam1.setText(formattedData);
      sam2.setText(formattedData);
      sam1.setBackgroundDrawable(getResources().getDrawable(R.drawable.box_background));
      sam2.setBackgroundDrawable(getResources().getDrawable(R.drawable.box_background));
    } else {
      Log.d(TAG, "closeTDA Status : FAILED");
      closeTda.setText("Close TDA FAILED");
    }
  }

  private void onDiscoverTdaClick() {

    TdaResult tdadiscoverStatus = new TdaResult();

    NfcTDAInfo[] mTdaInfo = mNxpNfcAdapter.discoverTDA(tdadiscoverStatus);

    if(tdadiscoverStatus.getStatus() == TdaResult.RESULT_SUCCESS) {
      discoverTda.setBackgroundColor(getResources().getColor(R.color.green));
      discoverTda.setText("TDA_DISCOVER_DONE");
      Log.d(TAG, "Num of TDAs Info Available = " +mTdaInfo.length);
      for(int i = 0; i < mTdaInfo.length; i++)
      {
        displayTdaInfoOnUI(mTdaInfo[i], i);
        byte tdaID = mTdaInfo[i].id;
        String hextdaID = String.format("%02X", tdaID);
        Log.d(TAG, "\nTDA ID : 0x" +hextdaID + "\nStatus : " +mTdaInfo[i].status + "\nNo. of protocols " +mTdaInfo[i].numberOfProtocols);

        for(int j = 0; j < mTdaInfo[i].numberOfProtocols; j++)
        {
          Log.d(TAG, "protocols " +mTdaInfo[i].protocols[j]);
        }

        Log.d(TAG, " numberOfCardInfo " +mTdaInfo[i].numberOfCardInfo);

        for(int k = 0; k < mTdaInfo[i].numberOfCardInfo; k++)
        {
          Log.d(TAG, " cardTLVInfo type " +mTdaInfo[i].cardTLVInfo[k].type);
          Log.d(TAG, " cardTLVInfo length " +mTdaInfo[i].cardTLVInfo[k].length);

          for (int l = 0; l < mTdaInfo[i].cardTLVInfo[k].length; l++) {
              byte value = mTdaInfo[i].cardTLVInfo[k].value[l];
              String hexValue = String.format("%02X", value);
              Log.d(TAG, " value[" + l + "] = 0x" + hexValue);
          }
        }
      }
    } else {
      discoverTda.setText("TDA_DISCOVER_FAILED");
      Log.d(TAG, "discoverTDA Status : FAILED");
    }
    Log.d(TAG, "onDiscoverTdaClick: ");
  }

  private final CompoundButton.OnCheckedChangeListener onCheckedChangeListener =
      new CompoundButton.OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView,
                                     boolean isChecked) {
          final int buttonId = buttonView.getId();
          switch (buttonId) {
          case R.id.tda_SAM1:
            onTdaSam1Click(isChecked);
            break;
          case R.id.tda_SAM2:
            onTdaSam2Click(isChecked);
            break;
          }
        }
      };

    private final View.OnClickListener onClickListener =
      new View.OnClickListener() {
        @Override
        public void onClick(View v) {
          switch (v.getId()) {
                case R.id.discoverTda:
                      onDiscoverTdaClick();
                      break;
                case R.id.trnsSend:
                      onSendTrnsClick();
                      break;
                case R.id.closeTda:
                      onCloseTdaClick();
                      break;
                case R.id.openTda:
                      onOpenTdaClick();
                      break;
          }
        }
    };
  @Override
  protected void onStop() {
    super.onStop();
  }
}
