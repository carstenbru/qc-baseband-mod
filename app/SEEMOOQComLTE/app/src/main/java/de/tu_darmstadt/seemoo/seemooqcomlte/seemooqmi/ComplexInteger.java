/**
 * class for complex integers
 *
 * @author Carsten Bruns (carst.bruns@gmx.de)
 */

package de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi;

public class ComplexInteger {
    private int real;
    private int imaginary;

    /**
     * constructor
     *
     * @param real real part of the number
     * @param imaginary imaginary part of the number
     */
    public ComplexInteger(int real, int imaginary) {
        this.real = real;
        this.imaginary = imaginary;
    }

    public int getReal() {
        return real;
    }

    public int getImaginary() {
        return imaginary;
    }

    public double abs() {
        return  Math.sqrt(real*real + imaginary*imaginary);
    }

    /**
     * returns the numbers angle in degrees (0-360)
     *
     * @return angle in degrees
     */
    public double angleInDegree() {
        double angle = 360.0 * Math.atan2(imaginary, real) / (2*Math.PI);
        return angle < 0 ? angle + 360.0 : angle;
    }
}
