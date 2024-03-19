from fspeclib import *

Function(
    name='ctlst.io.gpio_in',
    title=LocalizedString(
        en='gpio_in'
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
    inputs=[],
    outputs=[
        Output(
            name='output',
            title='Output',
            value_type='core.type.bool'
        )],
    state=[],
    parameter_constraints=[]
)
