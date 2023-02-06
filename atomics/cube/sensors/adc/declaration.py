from fspeclib import *

Function(
    name='cube.sensors.adc',
    title=LocalizedString(
        en='io'
    ),
    parameters=[
        Parameter(
            name='channel',
            title='ADC channel',
            value_type='core.type.u8',
            default=0,
        )],
    outputs=[
        Output(
            name='o',
            title='Current voltage',
            value_type='core.type.f64'
        ),
        Output(
            name='max',
            title='Max voltage between atomic calls',
            value_type='core.type.f64'
        ),
        Output(
            name='min',
            title='Min voltage between atomic calls',
            value_type='core.type.f64'
        ),
    ],
    parameter_constraints=[],
)
