import time
import serial
import pygame.mixer
import alsaaudio

SERIAL = serial.Serial('/dev/ttyACM0', 9600)

MIXER = alsaaudio.Mixer('PCM')
MIXER.setvolume(100)

NUM_CHANNELS = 32
pygame.mixer.pre_init(44100, -16, 1, 512)
pygame.mixer.init()
pygame.mixer.set_num_channels(NUM_CHANNELS)
print pygame.mixer.get_num_channels()

MUSIC = {
    '$M-1': '/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/Music/Mario-Gameover.mp3',
    '$M0': '/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/Music/Stayin-Alive.mp3',
    '$M1': '/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/Music/Hot-Stuff.mp3',
    '$M2': '/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/Music/Abba-Gimme.mp3',
    '$M3': '/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/Music/YMCA.mp3',
    '$M4': '/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/Music/Aerobic.mp3',
    '$M5': '/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/Music/Boogie-Wonderland.mp3'
}

SOUNDS = {
    '$S0': pygame.mixer.Sound('/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/UT/doublekill.wav'),
    '$S1': pygame.mixer.Sound('/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/SW-Blaster-new.wav'),
    '$S2': pygame.mixer.Sound('/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/hitmarker.wav'),
    '$S3': pygame.mixer.Sound('/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/PlayerOneWins.wav'),
    '$S4': pygame.mixer.Sound('/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/PlayerTwoWins.wav'),
    '$S5': pygame.mixer.Sound('/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/Round1.wav'),
    '$S6': pygame.mixer.Sound('/home/pi/src/LED-Disco-King-Game/Arduino-Raspberry-Connection/Sounds/Round2.wav'),
}

def play_music(serial_input):
    pygame.mixer.music.stop()
    pygame.mixer.music.load(MUSIC[serial_input])
    pygame.mixer.music.play(loops=-1)

current_channel = 0

def play_sound(serial_input):
    global current_channel
    pygame.mixer.Channel(current_channel).play(SOUNDS[serial_input])
    current_channel = (current_channel + 1) % NUM_CHANNELS

def play(serial_input):
    if serial_input.startswith('$M'):
        play_music(serial_input)
    elif serial_input.startswith('$S'):
        play_sound(serial_input)

while True:
    SERIAL_INPUT = SERIAL.readline().replace('\n', '').replace('\r', '')
    play(SERIAL_INPUT)
