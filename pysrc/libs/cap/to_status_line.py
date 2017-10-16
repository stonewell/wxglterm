import logging

def handle(term, context, cap_turple):
    mode = context.params[0] if len(context.params) > 0 else 0
    term.enter_status_line(mode, True)
