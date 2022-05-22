from ctlst import *

Function(
    name='ph.periph.px4io',
    title=LocalizedString(
        en='PX4IO'
    ),
    parameters=[
        Parameter(
            name='max_pwm',
            title='Max PWM in usec',
            value_type='core.type.f64',
            tunable=True,
            default=0,
            constraints=[
                ThisValue() >= 0
            ]
        ),
        Parameter(
            name='min_pwm',
            title='Min PWM in usec',
            value_type='core.type.f64',
            tunable=True,
            default=0,
            constraints=[
                ThisValue() >= 0
            ]
        ),
    ],
    inputs=[
        Input(
            name='arm',
            title='Arm/Disarm PX4IO',
            value_type='core.type.bool'
        ),
        Input(
            name='pwm_channel1',
            title='pwm channel1',
            value_type='core.type.f64'
        ),
        Input(
            name='pwm_channel2',
            title='pwm channel2',
            value_type='core.type.f64'
        ),
        Input(
            name='pwm_channel3',
            title='pwm channel3',
            value_type='core.type.f64'
        ),
        Input(
            name='pwm_channel4',
            title='pwm channel4',
            value_type='core.type.f64'
        ),
        Input(
            name='pwm_channel5',
            title='pwm channel5',
            value_type='core.type.f64'
        ),
        Input(
            name='pwm_channel6',
            title='pwm channel6',
            value_type='core.type.f64'
        ),
        Input(
            name='pwm_channel7',
            title='pwm channel7',
            value_type='core.type.f64'
        ),
        Input(
            name='pwm_channel8',
            title='pwm channel8',
            value_type='core.type.f64'
        ),
    ],
    outputs=[
        Output(
            name='rc_channel1',
            title='rc_channel1',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel2',
            title='rc_channel2',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel3',
            title='rc_channel3',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel4',
            title='rc_channel4',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel5',
            title='rc_channel5',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel6',
            title='rc_channel6',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel7',
            title='rc_channel7',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel8',
            title='rc_channel8',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel9',
            title='rc_channel9',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel10',
            title='rc_channel10',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel11',
            title='rc_channel11',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel12',
            title='rc_channel12',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel13',
            title='rc_channel13',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel14',
            title='rc_channel14',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel15',
            title='rc_channel16',
            value_type='core.type.u16'
        ),
        Output(
            name='rc_channel16',
            title='rc_channel16',
            value_type='core.type.u16'
        ),
    ],
    state=[
        Variable(
            name='gen_state',
            title='PX4IO General State',
            value_type='core.type.u16'
        ),
        Variable(
            name='arm_state',
            title='PX4IO Arming State',
            value_type='core.type.u16'
        ),
        Variable(
            name='rc_state',
            title='PX4IO Arming State',
            value_type='core.type.u16'
        ),
    ],
    parameter_constraints=[
    ],
    injection=Injection(
        timedelta=True
    )
)
