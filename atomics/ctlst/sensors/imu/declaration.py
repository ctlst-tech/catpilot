from fspeclib import *

Function(
    name='ctlst.sensors.imu',
    title=LocalizedString(
        en='imu'
    ),
    has_pre_exec_init_call=True,
    custom_cmakefile=True,
    parameters=[
        Parameter(
            name='freq',
            title='Polling frequency',
            value_type='core.type.u32',
            tunable=False,
            default=500
        ),
    ],
    inputs=[],
    outputs=[
        Output(
            name='ax',
            title='X-axis acceleration',
            value_type='core.type.f64'
        ),
        Output(
            name='ay',
            title='Y-axis acceleration',
            value_type='core.type.f64'
        ),
        Output(
            name='az',
            title='Z-axis acceleration',
            value_type='core.type.f64'
        ),
        Output(
            name='wx',
            title='X-axis angular rate',
            value_type='core.type.f64'
        ),
        Output(
            name='wy',
            title='Y-axis angular rate',
            value_type='core.type.f64'
        ),
        Output(
            name='wz',
            title='Z-axis angular rate',
            value_type='core.type.f64'
        ),
        Output(
            name='pstat',
            title='Static pressure',
            value_type='core.type.f64'
        ),
        Output(
            name='pdiff',
            title='Diff pressure',
            value_type='core.type.f64'
        ),
        Output(
            name='pdyn',
            title='Dynamic pressure',
            value_type='core.type.f64'
        ),
        Output(
            name='tadc',
            title='ADC temperature',
            value_type='core.type.f64'
        ),
        Output(
            name='tax',
            title='X-axis accel temperature',
            value_type='core.type.f64'
        ),
        Output(
            name='tay',
            title='Y-axis accel temperature',
            value_type='core.type.f64'
        ),
        Output(
            name='taz',
            title='Z-axis accel temperature',
            value_type='core.type.f64'
        ),
        Output(
            name='twx',
            title='X-axis gyro temperature',
            value_type='core.type.f64'
        ),
        Output(
            name='twy',
            title='Y-axis gyro temperature',
            value_type='core.type.f64'
        ),
        Output(
            name='twz',
            title='Z-axis gyro temperature',
            value_type='core.type.f64'
        ),
    ],
    state=[
        Variable(
            name='period',
            title='Period us',
            value_type='core.type.u32'
        ),
    ],
    parameter_constraints=[
        ParameterValue('freq') >= 1,
        ParameterValue('freq') <= 500,
    ]
)
