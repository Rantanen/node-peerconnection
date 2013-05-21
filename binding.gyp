# Build file for peerconnection
{
    'os_posix': 1,
    'variables': {
        # Default to libjingle being checked out next to the current project dir.
        # PRODUCT_DIR is one of the few absolute dirs we got.
        # TODO Do we have a variable for the absolute binding.gyp dir?
        'libjingle%': '<(PRODUCT_DIR)/../../../libjingle/trunk/',
        'target_arch%': 'x64',
    },
    'target_defaults': {
        'default_configuration': 'Release',
        'configuration': {
            'Debug': {
                'defines': [ 'DEBUG', '_DEBUG' ],
                'msvs_settings': {
                    'VSSLCompilerTool': {
                        'RuntimeLibrary': 1
                    },
                },
                'variables': {
                    'config%': 'Debug'
                }
            },
            'Release': {
                'defines': [ 'NODEBUG' ],
                'msvs_settings': {
                    'VSSLCompilerTool': {
                        'RuntimeLibrary': 0
                    },
                },
                'variables': {
                    'config%': 'Release'
                }
            },
        },
    },
    'conditions': [
        ['OS=="win"', {
            'variables': {
                'copy_command%': 'copy',
                'mkdir_command%': 'mkdir',
                'mkdir_params%':  '',
            }
        }, {
            'variables': {
                'copy_command%': 'cp',
                'mkdir_command%': 'mkdir',
                'mkdir_params%':  '-p',
            },
        }],
    ],
    'targets': [
        {
            'target_name': 'action_after_build',
            'type': 'none',
            'dependencies': [ 'peerconnection', 'ensure_output_dir' ],
            'actions': [
                {
                    'action_name': 'copy',
                    'inputs': [
                        '<(PRODUCT_DIR)/peerconnection.node',
                    ],
                    'outputs': [
                        '../compiled/<(target_arch)/<(OS)'
                    ],
                    'action': [ '<(copy_command)', '<(PRODUCT_DIR)/peerconnection.node', '../compiled/<(OS)/<(target_arch)/' ],
                }
            ]
        },
        {
            'target_name': 'ensure_output_dir',
            'type': 'none',
            'actions': [
                {
                    'action_name': 'mkdir',
                    'inputs': [
                        '<(PRODUCT_DIR)/peerconnection.node',
                    ],
                    'outputs': [
                        'compiled/<(target_arch)/<(OS)'
                    ],
                    'action': [ '<(mkdir_command)', '<(mkdir_params)', '../compiled/<(OS)/<(target_arch)/' ],
                },
            ]
        },
        {
            'target_name': 'peerconnection',
            'dependencies': [
            ],
            'variables': {
                'libjingle_out%': '<(libjingle)/out/Release/obj/',
            },
            'cflags': [
                '-pthread',
                '-fno-exceptions',
                '-fno-strict-aliasing',
                '-Wall',
                '-Wno-unused-parameter',
                '-Wno-missing-field-initializers',
                '-Wextra',
                '-pipe',
                '-fno-ident',
                '-fdata-sections',
                '-ffunction-sections',
                '-fPIC'
            ],
            'defines': [
                'LARGEFILE_SOURCE',
                '_FILE_OFFSET_BITS=64',
                'WEBRTC_TARGET_PC',
                'WEBRTC_LINUX',
                'WEBRTC_THREAD_RR',
                'EXPAT_RELATIVE_PATH',
                'GTEST_RELATIVE_PATH',
                'JSONCPP_RELATIVE_PATH',
                'WEBRTC_RELATIVE_PATH',
                'POSIX',
                '__STDC_FORMAT_MACROS',
                'DYNAMIC_ANNOTATIONS_ENABLED=0'
            ],
            'include_dirs': [
                '<(libjingle)',
                '<(libjingle)/third_party/webrtc',
                '<(libjingle)/third_party/webrtc/system_wrappers/interface',
                '<(libjingle)/third_party/',
            ],
            'sources': [
                'src/binding.cc',
                'src/peerconnection.cc',
                'src/callbackaudiodevice.cc',
            ],
            'link_settings': {
                'ldflags': [
                ],
                'libraries': [
                    '-Wl,-Bstatic',
                    '<(libjingle_out)/talk/libjingle_peerconnection.a',
                    '<(libjingle_out)/talk/libjingle_p2p.a',
                    '<(libjingle_out)/talk/libjingle_media.a',
                    '<(libjingle_out)/talk/libjingle_sound.a',
                    '<(libjingle_out)/talk/libjingle.a',
                    '<(libjingle_out)/third_party/webrtc/voice_engine/libvoice_engine_core.a',
                    '<(libjingle_out)/third_party/webrtc/video_engine/libvideo_engine_core.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libwebrtc_utility.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libaudio_conference_mixer.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libaudio_processing.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libaudio_processing_sse2.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libaudio_coding_module.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libaudio_device.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libvideo_processing.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libvideo_processing_sse2.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libvideo_render_module.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libremote_bitrate_estimator.a',
                    '<(libjingle_out)/third_party/webrtc/modules/librtp_rtcp.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libNetEq.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libwebrtc_opus.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libG711.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libiSAC.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libCNG.a',
                    '<(libjingle_out)/third_party/webrtc/modules/libaudioproc_debug_proto.a',
                    '<(libjingle_out)/third_party/webrtc/common_audio/libcommon_audio.a',
                    #'<(libjingle_out)/third_party/webrtc/common_audio/libresampler.a',
                    #'<(libjingle_out)/third_party/webrtc/common_audio/libsignal_processing.a',
                    #'<(libjingle_out)/third_party/webrtc/common_audio/libvad.a',
                    '<(libjingle_out)/third_party/libsrtp/libsrtp.a',
                    '<(libjingle_out)/third_party/opus/libopus.a',
                    '<(libjingle_out)/third_party/protobuf/libprotobuf_lite.a',
                    '<(libjingle_out)/third_party/webrtc/system_wrappers/source/libsystem_wrappers.a',
                    '-Wl,-Bdynamic'
                ]
            }
        },
    ]
}
