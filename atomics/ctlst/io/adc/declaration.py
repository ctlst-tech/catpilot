from fspeclib import *

Function(
    name='ctlst.io.adc',
    title=LocalizedString(
        en='adc'
    ),
    has_pre_exec_init_call=True,
    custom_cmakefile=True,
    parameters=[
        Parameter(
            name='channel',
            title='Channel number',
            value_type='core.type.u32',
            tunable=False,
            default=0
        ),
        Parameter(
            name='period',
            title='Period',
            value_type='core.type.u32',
            tunable=False,
            default=0
        ),
        Parameter(
            name='bias',
            title='Bias',
            value_type='core.type.f64',
            tunable=False,
            default=0
        ),
        Parameter(
            name='scale',
            title='Scale',
            value_type='core.type.f64',
            tunable=False,
            default=0
        ),
    ],
    inputs=[],
    outputs=[
        Output(
            name='output',
            title='Output',
            value_type='core.type.f64'
        ),
    ],
    state=[
    ],
    parameter_constraints=[
    ]
)
