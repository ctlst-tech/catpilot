from fspeclib import *

Function(
    name='ctlst.io.gpio_out',
    title=LocalizedString(
        en='gpio_out'
    ),
    has_pre_exec_init_call=True,
    custom_cmakefile=True,
    parameters=[
        Parameter(
            name='channel',
            title='GPIO channel',
            value_type='core.type.u32',
            tunable=False,
            default=0
        ),
    ],
    inputs=[
        Input(
            name='input_bool',
            title='input bool',
            value_type='core.type.bool',
            mandatory=False,
        ),
        Input(
            name='input_float',
            title='input float 64',
            value_type='core.type.f64',
            mandatory=False
        ),
    ],
    outputs=[],
    state=[],
    parameter_constraints=[]
)
