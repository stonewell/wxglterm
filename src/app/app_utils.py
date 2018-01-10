import sys
import os


app_name = 'wxglterm'
config_file_ext = 'json'
config_file_name = '.'.join([app_name, config_file_ext])

if sys.platform == 'win32':
    raise ValueError('windows not supported!')


def user_config_dir():
    if sys.platform == 'darwin':
        p = os.path.expanduser('~/Library/Preferences')
    else:
        p = os.getenv('XDG_CONFIG_HOME',
                      os.path.expanduser('~/.config'))

    return [os.path.join(p, app_name)]


def sys_config_dir():
    if sys.platform == 'darwin':
        p = os.path.expanduser('/Library/Preferences')

        return [os.path.join(p, app_name)]
    else:
        return [os.path.join('/usr/local/share', app_name),
                os.path.join('/usr/share', app_name)]


def find_config_file():
    p = os.getenv('WXGLTERM_CONFIG_PATH')

    if p:
        return p

    dirs = []
    dirs.extend(user_config_dir())
    dirs.extend(sys_config_dir())

    for x in dirs:
        p = os.path.join(x,
                         config_file_name)

        if os.path.isfile(p):
            return p

    return config_file_name
