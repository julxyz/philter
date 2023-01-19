import numpy as np
import sounddevice
import signal
from time import sleep

SAMPLING_RATE = 44100
DURATION = 10
AMPLITUDE = 0.3

samples = int(DURATION * SAMPLING_RATE)

# generate white noise
input_signal = np.random.default_rng().uniform(-1,1, samples)

cutoff = np.geomspace(SAMPLING_RATE/2, 20, int(samples/2))
cutoff = np.append(cutoff, cutoff[::-1])

# output buffer
allpass_out = np.zeros_like(input_signal)

# allpass buffer
dn_1 = 0

for n in range(samples):
    break_freq = cutoff[n]

    # allpass coefficient
    tan = np.tan(np.pi * break_freq / SAMPLING_RATE)
    ap_coeff = (tan-1) / (tan+1)

    # apply to current sample
    allpass_out[n] = ap_coeff * input_signal[n] + dn_1

    dn_1 = input_signal[n] - ap_coeff * allpass_out[n]

# combine allpass with direct in for low pass
output_signal = (input_signal + allpass_out)/2

output_signal *= AMPLITUDE


# loop playback
def handler(signum, frame):
    sounddevice.stop()
    exit(0)
 
signal.signal(signal.SIGINT, handler)

sounddevice.play(output_signal, SAMPLING_RATE, loop=True)

while True:
    sleep(0.1)
