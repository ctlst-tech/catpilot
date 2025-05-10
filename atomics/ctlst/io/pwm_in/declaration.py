from fspeclib import *

Function(
    name='ctlst.io.pwm_in',
    title=LocalizedString(
        en='pwm'
    ),
    has_pre_exec_init_call=True,
    custom_cmakefile=True,
    parameters=[
        Parameter(
            name='channel',
            title='PWM channel',
            value_type='core.type.u32',
            tunable=False,
            default=0
        ),
        Parameter(
            name='min',
            title='Channel min PWM',
            value_type='core.type.u32',
            default=700,
        ),
        Parameter(
            name='max',
            title='Channel max PWM',
            value_type='core.type.u32',
            default=700,
        ),
        Parameter(
            name='bipolar',
            title='Bipolar mode enable',
            value_type='core.type.bool',
            default='FALSE',
        ),
    ],
    inputs=[],
    outputs=[
        Output(
            name='period',
            title='Period in microseconds',
            value_type='core.type.f64'
        ),
        Output(
            name='duty',
            title='Duty',
            value_type='core.type.f64'
        )],
    state=[],
    parameter_constraints=[]
)
