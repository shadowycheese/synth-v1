import math
import struct

def generate_perfect_wavetable(num_samples=256):
    hex_string = ""
    
    for i in range(num_samples):
        # By distributing the phase across 256 intervals, 
        # sample 255 smoothly resolves right before 2*pi
        #angle = (2.0 * math.pi * i) / num_samples

        # If your Teensy synth prefers a true 0-crossing boundary:
        angle = (2.0 * math.pi * i) / (num_samples - 1)
        
        sine_val = math.sin(angle)
        scaled_val = round(sine_val * 32767)
        
        binary_data = struct.pack('>h', scaled_val)
        hex_string += binary_data.hex().upper()
        
    return hex_string

if __name__ == "__main__":
    result = generate_perfect_wavetable(256)
    print(result)