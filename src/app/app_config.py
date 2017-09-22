import os
import json

class DictQuery(dict):
    def get(self, path, default = None):
        keys = path.split("/")
        val = None

        for key in keys:
            #skip empty keys for // and path start with /
            if len(key) == 0:
                continue
            if val:
                if isinstance(val, list):
                    val = [ v.get(key, default) if v else None for v in val]
                else:
                    val = val.get(key, default)
            else:
                val = dict.get(self, key, default)

            if not val:
                break;

        return val

def load_config(config_path):
    if not os.path.exists(config_path):
        raise ValueError('unable to find the config file:{}'.format(config_path))

    with open(config_path) as f:
        return DictQuery(json.load(f))
