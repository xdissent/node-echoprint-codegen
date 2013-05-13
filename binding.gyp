{
  "targets": [
    {
      "target_name": "ecg",
      "sources": [ "ecg.cc" ],
      "dependencies": [
        "codegen",
        "echoprint-codegen"
      ]
    },
    {
      "target_name": "codegen",
      "type": "static_library",
      "product_prefix": "lib",
      "sources": [
        "echoprint-codegen/src/Codegen.cxx", "echoprint-codegen/src/AudioBufferInput.cxx", "echoprint-codegen/src/AudioStreamInput.cxx", "echoprint-codegen/src/MatrixUtility.cxx", "echoprint-codegen/src/Fingerprint.cxx", "echoprint-codegen/src/Whitening.cxx", "echoprint-codegen/src/SubbandAnalysis.cxx", "echoprint-codegen/src/Base64.cxx"
      ],
      "direct_dependent_settings": {
        "include_dirs": [ "echoprint-codegen/src" ],
        "libraries": [ "-lcodegen" ]
      },
      "include_dirs": [
        "<!@(pkg-config taglib --cflags-only-I | sed s/-I//g)"
      ],
      "libraries": [
        "<!@(pkg-config taglib --libs)",
        "-lz"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions", "-Wno-overloaded-virtual" ],
      "conditions": [
        ["OS=='mac'", {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "WARNING_CFLAGS": [ "-Wno-overloaded-virtual" ]
          }
        }]
      ]
    },
    {
      "target_name": "echoprint-codegen",
      "type": "executable",
      "dependencies": [ "codegen" ],
      "sources": [
        "echoprint-codegen/src/main.cxx", "echoprint-codegen/src/Metadata.cxx"
      ],
      "include_dirs": [
        "<!@(pkg-config taglib --cflags-only-I | sed s/-I//g)"
      ],
      "libraries": [
        "<!@(pkg-config taglib --libs)"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions", "-Wno-overloaded-virtual" ],
      "conditions": [
        ["OS=='mac'", {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "WARNING_CFLAGS": [ "-Wno-overloaded-virtual" ]
          }
        }]
      ]
    }
  ]
}