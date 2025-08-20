/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA
 *
 *  Author : Deon van der Westhuysen - June 2012 - Vodacom PTY LTD
 */

#ifndef __RTPSTREAM__
#define __RTPSTREAM__

#include <unordered_map>

#define RTPSTREAM_MAX_FILENAMELEN 256
#define RTPSTREAM_MAX_PAYLOADNAME 256
#define RTPECHO_MAX_FILENAMELEN 256
#define RTPECHO_MAX_PAYLOADNAME 256

class JLSRTP;

#ifdef USE_TLS
struct SrtpAudioInfoParams
{
    bool audio_found = false;
    int primary_audio_cryptotag = 0;
    char primary_audio_cryptosuite_store[25] = "";
    std::string_view primary_audio_cryptosuite;
    char primary_audio_cryptokeyparams[42] = "";
    int secondary_audio_cryptotag = 0;
    char secondary_audio_cryptosuite_store[25] = "";
    std::string_view secondary_audio_cryptosuite;
    char secondary_audio_cryptokeyparams[42] = "";
    bool primary_unencrypted_audio_srtp = false;
    bool secondary_unencrypted_audio_srtp = false;
};

struct SrtpVideoInfoParams
{
    bool video_found = false;
    int primary_video_cryptotag = 0;
    char primary_video_cryptosuite_store[25] = "";
    std::string_view primary_video_cryptosuite;
    char primary_video_cryptokeyparams[42] = "";
    int secondary_video_cryptotag = 0;
    char secondary_video_cryptosuite_store[25] = "";
    std::string_view secondary_video_cryptosuite;
    char secondary_video_cryptokeyparams[42] = "";
    bool primary_unencrypted_video_srtp = false;
    bool secondary_unencrypted_video_srtp = false;
};
#endif // USE_TLS

struct threaddata_t;

struct taskentry_t
{
    ~taskentry_t();
    threaddata_t         *parent_thread = nullptr;
    unsigned long        nextwake_ms = 0;
    volatile int         flags = 0;

    /* rtp stream information */
    unsigned long long   last_audio_timestamp = 0;
    unsigned long long   last_video_timestamp = 0;
    unsigned short       audio_seq_out = 0;
    unsigned short       video_seq_out = 0;
    char                 audio_payload_type = 0;
    char                 video_payload_type = 0;
    unsigned int         audio_ssrc_id = 0;
    unsigned int         video_ssrc_id = 0;

    /* current playback information */
    int                  audio_pattern_id = -1; // FILE:  -1 (UNUSED) -- PATTERN: <id>
    int                  video_pattern_id = -1; // FILE:  -1 (UNUSED) -- PATTERN: <id>
    int                  audio_loop_count = -1; // FILE:  <loopCount> -- PATTERN: -1 (UNUSED)
    int                  video_loop_count = -1; // FILE:  <loopCount> -- PATTERN: -1 (UNUSED)
    char                 *audio_file_bytes_start = nullptr;
    char                 *video_file_bytes_start = nullptr;
    char                 *audio_current_file_bytes = nullptr;
    char                 *video_current_file_bytes = nullptr;
    int                  audio_file_num_bytes = 0;
    int                  video_file_num_bytes = 0;
    int                  audio_file_bytes_left = 0;
    int                  video_file_bytes_left = 0;

    /* playback timing information */
    int                  audio_ms_per_packet = 0;
    int                  video_ms_per_packet = 0;
    int                  audio_bytes_per_packet = 0;
    int                  video_bytes_per_packet = 0;
    int                  audio_timeticks_per_packet = 0;
    int                  video_timeticks_per_packet = 0;
    int                  audio_timeticks_per_ms = 0;
    int                  video_timeticks_per_ms = 0;

    /* new file playback information */
    int                  new_audio_pattern_id = -1; // FILE:  -1 (UNUSED) -- PATTERN: <id>
    int                  new_video_pattern_id = -1; // FILE:  -1 (UNUSED) -- PATTERN: <id>
    char                 new_audio_payload_type = 0; // FILE/PATTERN: <payload_type> (e.g. 0, 8, 96)
    char                 new_video_payload_type = 0; // FILE/PATTERN: <payload_type> (e.g. 0, 8, 96)
    int                  new_audio_loop_count = -1; // FILE:  <loopCount> -- PATTERN: -1 (UNUSED)
    int                  new_video_loop_count = -1; // FILE:  <loopCount> -- PATTERN: -1 (UNUSED)
    int                  new_audio_file_size = 0;
    int                  new_video_file_size = 0;
    char                 *new_audio_file_bytes = nullptr;
    char                 *new_video_file_bytes = nullptr;
    int                  new_audio_ms_per_packet = 0;
    int                  new_video_ms_per_packet = 0;
    int                  new_audio_bytes_per_packet = 0;
    int                  new_video_bytes_per_packet = 0;
    int                  new_audio_timeticks_per_packet = 0;
    int                  new_video_timeticks_per_packet = 0;

    /* sockets for audio/video rtp_rtcp */
    int                  audio_rtp_socket = 0;
    int                  audio_rtcp_socket = 0;
    int                  video_rtp_socket = 0;
    int                  video_rtcp_socket = 0;

#ifdef USE_TLS
    /* audio/video SRTP echo activity indicators */
    int                  audio_srtp_echo_active = 0;
    int                  video_srtp_echo_active = 0;
#endif // USE_TLS

    /* rtp peer address structures */
    struct sockaddr_storage    remote_audio_rtp_addr = {};
    struct sockaddr_storage    remote_audio_rtcp_addr = {};
    struct sockaddr_storage    remote_video_rtp_addr = {};
    struct sockaddr_storage    remote_video_rtcp_addr = {};

    /* we will have a mutex per call. should we consider refactoring to */
    /* share mutexes across calls? makes the per-call code more complex */

    /* thread mananagment structures */
    pthread_mutex_t      mutex = {};

    unsigned long        audio_comparison_errors = 0;
    unsigned long        video_comparison_errors = 0;

    int                  audio_active = 0;
    int                  video_active = 0;

#ifdef USE_TLS
    SrtpAudioInfoParams  local_srtp_audio_params;
    SrtpAudioInfoParams  remote_srtp_audio_params;
    SrtpVideoInfoParams  local_srtp_video_params;
    SrtpVideoInfoParams  remote_srtp_video_params;
#endif // USE_TLS
};

struct rtpstream_callinfo_t
{
    taskentry_t *taskinfo;
    int local_audioport;
    int local_videoport;
    int remote_audioport;
    int remote_videoport;
    pthread_t threadID;
};

struct rtpstream_actinfo_t
{
    char filename[RTPSTREAM_MAX_FILENAMELEN];   // FILE: "<filename>" -- PATTERN: "pattern"
    int pattern_id;                             // FILE:  -1 -- PATTERN:  <id>
    int loop_count;                             // FILE: count -- PATTERN:  -1 (UNUSED)
    int bytes_per_packet;
    int ms_per_packet;
    int ticks_per_packet; /* need rework for 11.025 sample rate */
    int payload_type;
    char payload_name[RTPSTREAM_MAX_PAYLOADNAME];    // FILE/PATTERN: <payload_name> (e.g. "PCMU/8000", "PCMA/8000", "G729/8000", "H264/90000")
    int audio_active;
    int video_active;
};

struct rtpecho_actinfo_t
{
    int    payload_type;
    char   payload_name[RTPECHO_MAX_PAYLOADNAME];    // e.g. "PCMU/8000", "PCMA/8000", "G729/8000", "H264/90000"
    int    bytes_per_packet;
    int    audio_active;
    int    video_active;
};

union ParamPass
{
    int i;
    void* p;
};

union ResultCheck
{
    int i;
    void* p;
};

int rtpstream_new_call(rtpstream_callinfo_t *callinfo);
void rtpstream_end_call(rtpstream_callinfo_t *callinfo);
int rtpstream_shutdown(std::unordered_map<pthread_t, std::string>& threadIDs);

int rtpstream_get_local_audioport(rtpstream_callinfo_t *callinfo);
int rtpstream_get_local_videoport(rtpstream_callinfo_t *callinfo);
void rtpstream_set_remote(rtpstream_callinfo_t* callinfo, int ip_ver, const char* ip_addr,
                          int audio_port, int video_port);

#ifdef USE_TLS
int rtpstream_set_srtp_audio_local(rtpstream_callinfo_t *callinfo, SrtpAudioInfoParams &p);
int rtpstream_set_srtp_audio_remote(rtpstream_callinfo_t *callinfo, SrtpAudioInfoParams &p);
int rtpstream_set_srtp_video_local(rtpstream_callinfo_t *callinfo, SrtpVideoInfoParams &p);
int rtpstream_set_srtp_video_remote(rtpstream_callinfo_t *callinfo, SrtpVideoInfoParams &p);
#endif // USE_TLS

int rtpstream_cache_file(char *filename,
                         int mode /* 0: FILE - 1: PATTERN */,
                         int id,
                         int bytes_per_packet,
                         int stream_type /* 0: AUDIO - 1: VIDEO */);
void rtpstream_play(rtpstream_callinfo_t *callinfo, rtpstream_actinfo_t *actioninfo);
void rtpstream_pause(rtpstream_callinfo_t *callinfo);
void rtpstream_resume(rtpstream_callinfo_t *callinfo);

void rtpstream_playapattern(rtpstream_callinfo_t *callinfo, rtpstream_actinfo_t *actioninfo, JLSRTP& txUACAudio, JLSRTP& rxUACAudio);
void rtpstream_pauseapattern(rtpstream_callinfo_t *callinfo);
void rtpstream_resumeapattern(rtpstream_callinfo_t *callinfo);

void rtpstream_playvpattern(rtpstream_callinfo_t *callinfo, rtpstream_actinfo_t *actioninfo, JLSRTP& txUACVideo, JLSRTP& rxUACVideo);
void rtpstream_pausevpattern(rtpstream_callinfo_t *callinfo);
void rtpstream_resumevpattern(rtpstream_callinfo_t *callinfo);

void rtpstream_audioecho_thread(void * param);
void rtpstream_videoecho_thread(void * param);

int rtpstream_rtpecho_startaudio(rtpstream_callinfo_t *callinfo, JLSRTP& rxUASAudio, JLSRTP& txUASAudio);
int rtpstream_rtpecho_updateaudio(rtpstream_callinfo_t *callinfo, JLSRTP& rxUASAudio, JLSRTP& txUASAudio);
int rtpstream_rtpecho_stopaudio(rtpstream_callinfo_t *callinfo);

int rtpstream_rtpecho_startvideo(rtpstream_callinfo_t *callinfo, JLSRTP& rxUASVideo, JLSRTP& txUASVideo);
int rtpstream_rtpecho_updatevideo(rtpstream_callinfo_t *callinfo, JLSRTP& rxUASVideo, JLSRTP& txUASVideo);
int rtpstream_rtpecho_stopvideo(rtpstream_callinfo_t *callinfo);


#endif
