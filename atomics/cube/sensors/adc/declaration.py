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
            constraints=[
                ThisValue() >= 0,
                ThisValue() <= 5
            ]
        ),
        Parameter(
            name='scale',
            title='Conversion scale factor',
            value_type='core.type.f64',
            default=1.0,
            constraints=[
                ThisValue() >= 0.0,
            ]
        ),
        Parameter(
            name='offset',
            title='Conversion offset',
            description='Conversion offset applied before factor scaling',
            value_type='core.type.f64',
            default=0.0,
            constraints=[
                ThisValue() >= 0.0,
            ]
        )
    ],
    outputs=[
        Output(
            name='output',
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
