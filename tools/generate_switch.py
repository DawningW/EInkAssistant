import json

with open("font/qweather-icons.json", 'r', encoding="UTF-8") as f:
    icons = json.load(f)
    print("switch (id) {")
    try:
        for key, value in icons.items():
            id = int(key)
            if id > 999: break
            exp = '    case %d: return ' % id
            key_fill = key + "-fill"
            if key_fill in icons:
                exp += '!fill ? "\\u%x" : "\\u%x"' % (int(value), int(icons[key_fill]))
            else:
                exp += '"\\u%x"' % value
            exp += ';'
            print(exp)
    except:
        pass
    print("}")
