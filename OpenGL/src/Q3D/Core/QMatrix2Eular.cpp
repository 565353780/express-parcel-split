#include "QMatrix2Eular.h"
#include <cmath>
namespace GCL {

QMatrix2Euler::QMatrix2Euler()
{

}
float clamp(float x, float l, float r)
{
    return x<l?l:(x>r?r:x);
}
QVector3D QMatrix2Euler::fromMatrixToEuler(const QMatrix4x4 &m, const QString &order)
{

    float m11 = m(0,0);
    float m12 = m(1,0);
    float m13 = m(2,0);
    float m21 = m(0,1);
    float m22 = m(1,1);
    float m23 = m(2,1);
    float m31 = m(0,2);
    float m32 = m(1,2);
    float m33 = m(2,2);

    float x,y,z;
    if ( order == "XYZ" ) {

        y= asin( clamp( m13, - 1, 1 ) );

        if (fabs( m13 ) < 0.99999 ) {

            x = atan2( - m23, m33 );
            z = atan2( - m12, m11 );

        } else {

            x = atan2( m32, m22 );
            z = 0;

        }

    } else if ( order == "YXZ" ) {

        x = asin( - clamp( m23, - 1, 1 ) );

        if (fabs( m23 ) < 0.99999 ) {

            y= atan2( m13, m33 );
            z = atan2( m21, m22 );

        } else {

            y= atan2( - m31, m11 );
            z = 0;

        }

    } else if ( order == "ZXY" ) {

        x = asin( clamp( m32, - 1, 1 ) );

        if (fabs( m32 ) < 0.99999 ) {

            y= atan2( - m31, m33 );
            z = atan2( - m12, m22 );

        } else {

            y= 0;
            z = atan2( m21, m11 );

        }

    } else if ( order == "ZYX" ) {

        y= asin( - clamp( m31, - 1, 1 ) );

        if (fabs( m31 ) < 0.99999 ) {

            x = atan2( m32, m33 );
            z = atan2( m21, m11 );

        } else {

            x = 0;
            z = atan2( - m12, m22 );

        }

    } else if ( order == "YZX" ) {

        z = asin( clamp( m21, - 1, 1 ) );

        if (fabs( m21 ) < 0.99999 ) {

            x = atan2( - m23, m22 );
            y= atan2( - m31, m11 );

        } else {

            x = 0;
            y= atan2( m13, m33 );

        }

    } else if ( order == "XZY" ) {

        z = asin( - clamp( m12, - 1, 1 ) );

        if (fabs( m12 ) < 0.99999 ) {

            x = atan2( m32, m22 );
            y= atan2( m13, m11 );

        } else {

            x = atan2( - m23, m33 );
            y= 0;

        }

    }
    return -QVector3D(x,y,z) / 3.1415926358979 * 180;
}
}
