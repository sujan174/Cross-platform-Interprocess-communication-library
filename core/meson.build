project('miniipc', 'c',
        default_options: 'default_library=static')

# Define dependencies
kernel32_dep = dependency('kernel32')
user32_dep = dependency('user32')
advapi32_dep = dependency('advapi32')

executable('miniipc', 'src/main.c', 
    dependencies: [
        'named_pipe.c',
        'ordinary_pipe.c',
        'shared_memory.c',
        kernel32_dep,
        user32_dep,
        advapi32_dep
    ],
    include_directories: 'include'
)