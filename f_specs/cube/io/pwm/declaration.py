from ctlst import *

Function(
    name='cube.io.pwm',
    title=LocalizedString(
        en='io'
    ),
    parameters=[
        Parameter(
            name='ch1_min',
            title='channel',
            value_type='core.type.u32',
            default=700,
        ),
        Parameter(
            name='ch2_min',
            title='channel',
            value_type='core.type.u32',
            default=700,
        ),
        Parameter(
            name='ch3_min',
            title='channel',
            value_type='core.type.u32',
            default=700,
        ),
        Parameter(
            name='ch4_min',
            title='channel',
            value_type='core.type.u32',
            default=700,
        ),
        Parameter(
            name='ch5_min',
            title='channel',
            value_type='core.type.u32',
            default=700,
        ),
        Parameter(
            name='ch6_min',
            title='channel',
            value_type='core.type.u32',
            default=700,
        ),
        Parameter(
            name='ch7_min',
            title='channel',
            value_type='core.type.u32',
            default=700,
        ),
        Parameter(
            name='ch8_min',
            title='channel',
            value_type='core.type.u32',
            default=700,
        ),
        Parameter(
            name='ch1_max',
            title='channel',
            value_type='core.type.u32',
            default=2200,
        ),
        Parameter(
            name='ch2_max',
            title='channel',
            value_type='core.type.u32',
            default=2200,
        ),
        Parameter(
            name='ch3_max',
            title='channel',
            value_type='core.type.u32',
            default=2200,
        ),
        Parameter(
            name='ch4_max',
            title='channel',
            value_type='core.type.u32',
            default=2200,
        ),
        Parameter(
            name='ch5_max',
            title='channel',
            value_type='core.type.u32',
            default=2200,
        ),
        Parameter(
            name='ch6_max',
            title='channel',
            value_type='core.type.u32',
            default=2200,
        ),
        Parameter(
            name='ch7_max',
            title='channel',
            value_type='core.type.u32',
            default=2200,
        ),
        Parameter(
            name='ch8_max',
            title='channel',
            value_type='core.type.u32',
            default=2200,
        ),
        Parameter(
            name='ch1_bipolar',
            title='channel',
            value_type='core.type.bool',
            default='FALSE',
        ),
        Parameter(
            name='ch2_bipolar',
            title='channel',
            value_type='core.type.bool',
            default='FALSE',
        ),
        Parameter(
            name='ch3_bipolar',
            title='channel',
            value_type='core.type.bool',
            default='FALSE',
        ),
        Parameter(
            name='ch4_bipolar',
            title='channel',
            value_type='core.type.bool',
            default='FALSE',
        ),
        Parameter(
            name='ch5_bipolar',
            title='channel',
            value_type='core.type.bool',
            default='FALSE',
        ),
        Parameter(
            name='ch6_bipolar',
            title='channel',
            value_type='core.type.bool',
            default='FALSE',
        ),
        Parameter(
            name='ch7_bipolar',
            title='channel',
            value_type='core.type.bool',
            default='FALSE',
        ),
        Parameter(
            name='ch8_bipolar',
            title='channel',
            value_type='core.type.bool',
            default='FALSE',
        ),
    ],
    inputs=[
        Input(
            name='arm',
            title='CUBE IO PWM output arming signal',
            value_type='core.type.bool'
        ),
        Input(
            name='ch1',
            title='Channel 1 input',
            value_type='core.type.f64',
            mandatory=False
        ),
        Input(
            name='ch2',
            title='Channel 2 input',
            value_type='core.type.f64',
            mandatory=False
        ),
        Input(
            name='ch3',
            title='Channel 3 input',
            value_type='core.type.f64',
            mandatory=False
        ),
        Input(
            name='ch4',
            title='Channel 4 input',
            value_type='core.type.f64',
            mandatory=False
        ),
        Input(
            name='ch5',
            title='Channel 5 input',
            value_type='core.type.f64',
            mandatory=False
        ),
        Input(
            name='ch6',
            title='Channel 6 input',
            value_type='core.type.f64',
            mandatory=False
        ),
        Input(
            name='ch7',
            title='Channel 7 input',
            value_type='core.type.f64',
            mandatory=False
        ),
        Input(
            name='ch8',
            title='Channel 8 input',
            value_type='core.type.f64',
            mandatory=False
        ),
    ],
    outputs=[
        Output(
            name='stub',
            title='stub',
            value_type='core.type.f64'
        ),
    ],
    state=[],
    parameter_constraints=[],
)
