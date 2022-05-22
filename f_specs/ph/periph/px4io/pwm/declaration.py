from ctlst import *

Function(
    name='ph.periph.px4io.pwm',
    title=LocalizedString(
        en='PX4IO.pwm'
    ),
    parameters=[
        # Parameter(
        #     name='max_pwm',
        #     title='Max PWM in usec',
        #     value_type='core.type.i32',
        #     tunable=True,
        #     default=0,
        #     constraints=[
        #         ThisValue() >= 2000
        #     ]
        # ),
        # Parameter(
        #     name='min_pwm',
        #     title='Min PWM in usec',
        #     value_type='core.type.i32',
        #     tunable=True,
        #     default=0,
        #     constraints=[
        #         ThisValue() >= 1000
        #     ]
        # ),
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
            value_type='core.type.f32'
        ),
        Input(
            name='pwm_channel2',
            title='pwm channel2',
            value_type='core.type.f32'
        ),
        Input(
            name='pwm_channel3',
            title='pwm channel3',
            value_type='core.type.f32'
        ),
        Input(
            name='pwm_channel4',
            title='pwm channel4',
            value_type='core.type.f32'
        ),
        Input(
            name='pwm_channel5',
            title='pwm channel5',
            value_type='core.type.f32'
        ),
        Input(
            name='pwm_channel6',
            title='pwm channel6',
            value_type='core.type.f32'
        ),
        Input(
            name='pwm_channel7',
            title='pwm channel7',
            value_type='core.type.f32'
        ),
        Input(
            name='pwm_channel8',
            title='pwm channel8',
            value_type='core.type.f32'
        ),
    ],
    outputs=[
        Output(
            name='stub',
            title='stub',
            value_type='core.type.f32'
        ),
    ],
    state=[],
    parameter_constraints=[],
)
