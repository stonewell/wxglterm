import logging
import re
import sys


class Cap:
    def __init__(self):
        self.flags = {}
        self.cmds = {}
        self.control_data_start_state = ControlDataState()


def parse_cap(cap_str):
    cap = Cap()

    for field in cap_str.split(':'):
        if len(field) == 0:
            continue

        if field.find('=') > 0:
            cap.cmds.update(parse_str_cap(field, cap.control_data_start_state))
        elif field.find('#') > 0:
            parts = field.split('#')
            cap.flags.update({parts[0]: int(parts[1])})
        else:
            cap.flags.update({field: 1})

    return cap


class ControlDataParserContext:
    def __init__(self):
        self.params = []

    def push_param(self, param):
        self.params.append(param)


class ControlDataState:
    def __init__(self):
        self.cap_name = {}
        self.next_states = {}
        self.digit_state = None
        self.any_state = None

    def reset(self):
        pass

    def add_state(self, c, state):
        if c in self.next_states:
            return self.next_states[c]

        self.next_states[c] = state
        return state

    def add_digit_state(self, state):
        if self.digit_state:
            return self.digit_state

        self.digit_state = state
        return state

    def add_any_state(self, state):
        if self.any_state:
            return self.any_state

        self.any_state = state
        return state

    def handle(self, context, c):
        if c in self.next_states:
            next_state = self.next_states[c]
            if next_state.digit_state:
                next_state.digit_state.reset()

            return next_state

        if self.digit_state:
            next_state = self.digit_state.handle(context, c)

            if next_state:
                return next_state

        if self.any_state:
            return self.any_state.handle(context, c)

        return None

    def get_cap(self, params):
        if len(params) == 0:
            if '' in self.cap_name:
                return self.cap_name['']

            if '***' in self.cap_name:
                return self.cap_name['***']

            return None

        str_match = ','.join([str(x) for x in params])

        if str_match in self.cap_name:
            return self.cap_name[str_match]

        for k in sorted(self.cap_name, key=lambda v: str(v.count('*')) + v):
            if k.find('*') < 0:
                if k == str_match:
                    return self.cap_name[k]
                else:
                    continue

            re_str = k.replace('***', '(.:)')
            re_str = re_str.replace(',**', '(,[0-9]+)?')
            re_str = re_str.replace('**', '([0-9]+)?')
            re_str = re_str.replace('*', '[0-9]+')
            re_str = re_str.replace('?', '*')
            re_str = re_str.replace(':', '*')

            if re.match(re_str, str_match):
                return self.cap_name[k]

        return None


class DigitState(ControlDataState):
    def __init__(self):
        ControlDataState.__init__(self)
        self.digit_base = 10
        self.digits = ['0', '1', '2', '3', '4', '5',
                       '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F']
        self.value = None

    def handle(self, context, cc):
        c = cc.upper()

        if c in self.digits[:self.digit_base]:
            self.value = self.value * self.digit_base + \
                self.digits.index(c) \
                if self.value else self.digits.index(c)

            return self
        else:
            if self.value is not None:
                context.push_param(self.value)
                self.value = None
            return ControlDataState.handle(self, context, cc)

    def reset(self):
        self.value = None


class AnyState(ControlDataState):
    def __init__(self):
        ControlDataState.__init__(self)

    def handle(self, context, cc):
        next_state = ControlDataState.handle(self, context, cc)

        if next_state:
            return next_state

        context.push_param(cc)

        return self


class CapStringValue:
    def __init__(self):
        self.padding = 0.0
        self.value = ''
        self.name = ''

    def __str__(self):
        return ','.join([self.name, str(self.padding), self.value])

    def __repr__(self):
        return self.__str__()


def parse_padding(value):
    padding = 0.0
    pos = 0

    if value[0].isdigit():
        padding_chars = []
        has_dot = False

        for pos in range(len(value)):
            if value[pos] == '*':
                if len(padding_chars) > 0:
                    padding_chars.append('*')
                    pos += 1
                    break
            elif value[pos] == '.':
                if has_dot:
                    break
                has_dot = True
                padding_chars.append('.')
            elif value[pos].isdigit():
                padding_chars.append(value[pos])
            else:
                break
        #end for

        try:
            padding = 0.0
            if padding_chars[-1] == '*':
                padding = float(''.join(padding_chars[:-1]))
            else:
                padding = float(''.join(padding_chars))
        except ValueError:
            pass

    return (pos, padding)


def build_parser_state_machine(cap_str_value, start_state):
    value = cap_str_value.value

    pos = 0

    cur_state = start_state
    repeat_state = None
    repeat_char = None
    is_repeat_state = False
    repeat_enter_state = None

    increase_param = False
    is_digit_state = False
    digit_base = 10
    params = []
    cap_value = []

    while pos <len(value):
        c = value[pos]

        if c == '\\':
            pos += 1

            if pos >= len(value):
                raise ValueError("Unterminaled str")

            c = value[pos]
            if c == 'E':
                c = chr(0x1B)
            elif c == '\\':
                c = '\\'
            elif c == '(':
                is_repeat_state = True
                repeat_enter_state = cur_state
                pos += 1
                continue
            elif c == ')':
                if not repeat_state or not repeat_char:
                    raise ValueError("Invalid repeat state:" + str(pos) + "," + value)

                cur_state.add_state(repeat_char, repeat_state)

                repeat_char = None
                repeat_state = None
                is_repeat_state = False
                pos += 1
                continue
            elif c.isdigit():
                v = 0
                while pos < len(value) and c.isdigit():
                    v = v * 8 + int(c)
                    pos += 1
                    if pos < len(value):
                        c = value[pos]

                if not c.isdigit():
                    pos -= 1

                c = chr(v)
            elif c == '.':
                cur_state = cur_state.add_any_state(AnyState())
                params.append('***')
                pos += 1
                continue
            else:
                raise ValueError("unknown escape string:" + c + "," + str(pos) + "," + value)
        elif c == '^':
            pos += 1

            if pos >= len(value):
                raise ValueError("Unterminaled str")

            c = chr(ord(value[pos]) - ord('A') + 1)
        elif c == '%':
            pos += 1

            if pos >= len(value):
                raise ValueError("Unterminaled str")

            c = value[pos]

            if c == '%':
                c = '%'
            elif c == 'i':
                increase_param = True
                pos += 1
                continue
            elif c == 'd':
                is_digit_state = True
                digit_base = 10
            elif c == 'X' or c == 'x':
                is_digit_state = True
                digit_base = 16
            else:
                raise ValueError('unknown format string:' + c + "," + str(pos) + "," + value)
        elif c.isdigit():
            v = 0
            while pos < len(value) and c.isdigit():
                v = v * 10 + int(c)
                pos += 1
                if pos < len(value):
                    c = value[pos]

            if not c.isdigit():
                #restore the last digit
                pos -= 1
                #include all digit char into cap_value
                c = str(v)

            #save the params
            params.append(str(v))

        #build state with c
        if is_digit_state:
            cur_state = cur_state.add_digit_state(DigitState())
            cur_state.digit_base = digit_base
            #save the params
            if is_repeat_state:
                params.append('**')
            else:
                params.append('*')
        elif c.isdigit():
            cur_state = cur_state.add_digit_state(DigitState())
            cap_value.append(c)
        else:
            cur_state = cur_state.add_state(c, ControlDataState())
            cap_value.append(c)

        if is_repeat_state and not repeat_state:
            repeat_state = cur_state
            repeat_char = c

        if not is_repeat_state and repeat_enter_state:
            old_cur_state = cur_state
            cur_state = repeat_enter_state.add_state(c, cur_state)

            if cur_state != old_cur_state:
                #merge the state
                logging.error('should put generic pattern before special pattern')
                sys.exit(1)
            repeat_enter_state = None

        is_digit_state = False

        pos += 1

    return (cur_state, params, increase_param, ''.join(cap_value))

def parse_str_cap(field, start_state):
    cap_str_value = CapStringValue()

    parts = field.split('=')

    cap_str_value.name = parts[0]
    value = cap_str_value.value = '='.join(parts[1:])

    #padding
    pos, cap_str_value.padding = parse_padding(value)

    #build the parser state machine
    value = cap_str_value.value = value[pos:]

    cap_state, params, increase_param, cap_str_value.cap_value = build_parser_state_machine(cap_str_value, start_state)

    cap_name_key = ','.join(params)

    if cap_name_key in cap_state.cap_name:
        e_name, e_inc_param = cap_state.cap_name[cap_name_key]

        if (e_name != cap_str_value.name) or (e_inc_param != increase_param):
            raise ValueError('same parameter for different cap name:[' + cap_name_key + '],' + cap_str_value.name)

    cap_state.cap_name[cap_name_key] = (cap_str_value.name, increase_param)

    return {parts[0]:cap_str_value}
