// ==============================================================================
// ScarlettWarmth.dsp
// Analog Vocal Preamp with CM25 MkIII De-Harsh & Triode/Transformer Saturation
// Compatible with Faust IDE -> Export directly to VST3, AU, WebAudio, or C++
// ==============================================================================

declare name "Scarlett Warmth Preamp";
declare author "Analog Vocal DSP";
declare description "Vocal Preamp for Scarlett CM25 MkIII";

import("stdfaust.lib");

// --- CONTROLS ---
drive       = vslider("1. Input Drive [unit:dB]", 4.8, 0, 10, 0.1);
warmth      = vslider("2. Chest Warmth (220Hz) [unit:dB]", 5.5, 0, 10, 0.1);
cm25_tamer  = vslider("3. CM25 De-Harsh (5.8kHz) [unit:dB]", 6.5, 0, 10, 0.1);
iron        = vslider("4. Transformer Iron", 4.0, 0, 10, 0.1);
silk_air    = vslider("5. Silk Air (12kHz) [unit:dB]", 4.5, 0, 10, 0.1);
hpf_select  = nentry("6. High Pass [0:Flat, 1:40Hz, 2:80Hz, 3:120Hz]", 2, 0, 3, 1);
out_trim    = ba.db2linear(vslider("7. Output Trim [unit:dB]", 0.0, -12.0, 12.0, 0.2));
bypass      = checkbox("Bypass");

// --- HIGH PASS FILTER ---
// High pass frequency based on selector
hpf_freq = 20.0 + (hpf_select == 1) * 20.0 + (hpf_select == 2) * 60.0 + (hpf_select == 3) * 100.0;
hpf_stage = fi.highpass(2, hpf_freq);

// --- CM25 MkIII VOICE TAMER NOTCH ---
// Tames the aggressive +4dB peak at 5.8kHz in the CM25 capsule
notch_gain = cm25_tamer * -0.55;
cm25_filter = fi.peak_eq(notch_gain, 5800.0, 4200.0);

// --- WARMTH CHEST SHELF (220 Hz) ---
warmth_gain = warmth * 0.55;
warmth_filter = fi.peak_eq(warmth_gain, 220.0, 250.0);

// --- SILK AIR SHELF (12.5 kHz) ---
silk_gain = silk_air * 0.4;
silk_filter = fi.high_shelf(silk_gain, 12500.0);

// --- ASYMMETRIC TRIODE TUBE & TRANSFORMER SATURATION ---
triode_saturation(sig) = y
with {
  drive_norm = ma.fmax(0.05, drive / 10.0);
  iron_norm  = ma.fmax(0.0,  iron / 10.0);
  in_sig     = sig * (1.0 + drive_norm * 2.2);
  bias       = 0.28 * drive_norm;

  // Triode positive swing (smooth grid conduction)
  pos_out = ma.tanh(in_sig) + bias * (ma.tanh(in_sig) ^ 2) * 0.45;

  // Triode negative swing (soft cutoff)
  abs_s    = ma.fabs(in_sig);
  soft_neg = (0.0 - abs_s) / (1.0 + abs_s * 0.65);
  neg_out  = soft_neg - bias * (soft_neg ^ 2) * 0.20;

  tube_out = ba.if(in_sig >= 0.0, pos_out, neg_out);

  // Transformer magnetic core saturation
  iron_out = ma.tanh(in_sig * (1.0 + iron_norm * 1.8)) * 0.95;

  y = tube_out * (1.0 - iron_norm * 0.4) + iron_out * (iron_norm * 0.4);
};

// --- COMPLETE PROCESSOR ---
process_channel(x) = x 
                   : hpf_stage 
                   : cm25_filter 
                   : warmth_filter 
                   : silk_filter 
                   : triode_saturation 
                   : *(out_trim);

// Stereo routing with clean bypass
process(x, y) = (x * bypass + process_channel(x) * (1.0 - bypass)),
                (y * bypass + process_channel(y) * (1.0 - bypass));
