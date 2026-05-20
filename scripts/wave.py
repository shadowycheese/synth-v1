import math
import struct

def generate_perfect_wavetable(num_samples=256):
    hex_string = ""
    
    for i in range(num_samples):
        # By distributing the phase across 256 intervals, 
        # sample 255 smoothly resolves right before 2*pi
        #angle = (2.0 * math.pi * i) / num_samples

        multiplier = 2.0

        if i > 204:
            multiplier = 12.0
        elif i > 153:
            multiplier = 7.0
        elif i > 102:
            multiplier = 5.0
        elif i > 51:
            multiplier = 3.0
        
        
        # If your Teensy synth prefers a true 0-crossing boundary:
        angle = (multiplier * math.pi * i) / (num_samples - 1)
        
        sine_val = math.sin(angle)
        scaled_val = round(sine_val * 32767)
        
        binary_data = struct.pack('>h', scaled_val)
        hex_string += binary_data.hex().upper()
        
    return hex_string

if __name__ == "__main__":
    result = generate_perfect_wavetable(256)
    print(result)