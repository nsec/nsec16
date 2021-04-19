#pragma once

namespace music
{

// http://www.phy.mtu.edu/~suits/notefreqs.html
constexpr int C_0 = 1635 << 5;
constexpr int Db0 = 1732 << 5;
constexpr int D_0 = 1835 << 5;
constexpr int Eb0 = 1945 << 5;
constexpr int E_0 = 2060 << 5;
constexpr int F_0 = 2183 << 5;
constexpr int Gb0 = 2312 << 5;
constexpr int G_0 = 2450 << 5;
constexpr int Ab0 = 2596 << 5;
constexpr int A_0 = 2750 << 5;
constexpr int Bb0 = 2914 << 5;
constexpr int B_0 = 3087 << 5;
constexpr int C_1 = 3270 << 5;
constexpr int Db1 = 3465 << 5;
constexpr int D_1 = 3671 << 5;
constexpr int Eb1 = 3889 << 5;
constexpr int E_1 = 4120 << 5;
constexpr int F_1 = 4365 << 5;
constexpr int Gb1 = 4625 << 5;
constexpr int G_1 = 4900 << 5;
constexpr int Ab1 = 5191 << 5;
constexpr int A_1 = 5500 << 5;
constexpr int Bb1 = 5827 << 5;
constexpr int B_1 = 6174 << 5;
constexpr int C_2 = 6541 << 5;
constexpr int Db2 = 6930 << 5;
constexpr int D_2 = 7342 << 5;
constexpr int Eb2 = 7778 << 5;
constexpr int E_2 = 8241 << 5;
constexpr int F_2 = 8731 << 5;
constexpr int Gb2 = 9250 << 5;
constexpr int G_2 = 9800 << 5;
constexpr int Ab2 = 10383 << 5;
constexpr int A_2 = 11000 << 5;
constexpr int Bb2 = 11654 << 5;
constexpr int B_2 = 12347 << 5;
constexpr int C_3 = 13081 << 5;
constexpr int Db3 = 13859 << 5;
constexpr int D_3 = 14683 << 5;
constexpr int Eb3 = 15556 << 5;
constexpr int E_3 = 16481 << 5;
constexpr int F_3 = 17461 << 5;
constexpr int Gb3 = 18500 << 5;
constexpr int G_3 = 19600 << 5;
constexpr int Ab3 = 20765 << 5;
constexpr int A_3 = 22000 << 5;
constexpr int Bb3 = 23308 << 5;
constexpr int B_3 = 24694 << 5;
constexpr int C_4 = 26163 << 5;
constexpr int Db4 = 27718 << 5;
constexpr int D_4 = 29366 << 5;
constexpr int Eb4 = 31113 << 5;
constexpr int E_4 = 32963 << 5;
constexpr int F_4 = 34923 << 5;
constexpr int Gb4 = 36999 << 5;
constexpr int G_4 = 39200 << 5;
constexpr int Ab4 = 41530 << 5;
constexpr int A_4 = 44000 << 5;
constexpr int Bb4 = 46616 << 5;
constexpr int B_4 = 49388 << 5;
constexpr int C_5 = 52325 << 5;
constexpr int Db5 = 55437 << 5;
constexpr int D_5 = 58733 << 5;
constexpr int Eb5 = 62225 << 5;
constexpr int E_5 = 65926 << 5;
constexpr int F_5 = 69846 << 5;
constexpr int Gb5 = 73999 << 5;
constexpr int G_5 = 78399 << 5;
constexpr int Ab5 = 83061 << 5;
constexpr int A_5 = 88000 << 5;
constexpr int Bb5 = 93233 << 5;
constexpr int B_5 = 98777 << 5;
constexpr int C_6 = 104650 << 5;
constexpr int Db6 = 110873 << 5;
constexpr int D_6 = 117466 << 5;
constexpr int Eb6 = 124451 << 5;
constexpr int E_6 = 131851 << 5;
constexpr int F_6 = 139691 << 5;
constexpr int Gb6 = 147998 << 5;
constexpr int G_6 = 156798 << 5;
constexpr int Ab6 = 166122 << 5;
constexpr int A_6 = 176000 << 5;
constexpr int Bb6 = 186466 << 5;
constexpr int B_6 = 197553 << 5;
constexpr int C_7 = 209300 << 5;
constexpr int Db7 = 221746 << 5;
constexpr int D_7 = 234932 << 5;
constexpr int Eb7 = 248902 << 5;
constexpr int E_7 = 263702 << 5;
constexpr int F_7 = 279383 << 5;
constexpr int Gb7 = 295996 << 5;
constexpr int G_7 = 313596 << 5;
constexpr int Ab7 = 332244 << 5;
constexpr int A_7 = 352001 << 5;
constexpr int Bb7 = 372931 << 5;
constexpr int B_7 = 395107 << 5;
constexpr int C_8 = 418601 << 5;
constexpr int Db8 = 443492 << 5;
constexpr int D_8 = 469864 << 5;
constexpr int Eb8 = 497803 << 5;

enum class Music {
    NONE = 0,
    music_astronomia,
    music_mk,
    music_nyan,
    music_starwars,
    music_korobeiniki,
    sfx_dialog,
    sfx_failure,
    sfx_steps,
    sfx_success,
};

struct Playlist {
    Music id = Music::NONE;
    const int *music = nullptr;
    int bpm = 0;
    int repeat = 0;
};

} // namespace music

void buzzer_init();
void buzzer_request_music(music::Music music_id);
void buzzer_stop_music();
void buzzer_stop_music(music::Music music_id);