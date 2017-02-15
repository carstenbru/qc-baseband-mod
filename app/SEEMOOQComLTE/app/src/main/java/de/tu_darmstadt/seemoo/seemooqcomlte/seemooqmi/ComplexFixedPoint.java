package de.tu_darmstadt.seemoo.seemooqcomlte.seemooqmi;

//TODO fixed point treatments?
//TODO doc

public class ComplexFixedPoint {
    private int real;
    private int imaginary;

    public ComplexFixedPoint(int real, int imaginary) {
        this.real = real;
        this.imaginary = imaginary;
    }

    public int getReal() {
        return real;
    }

    public int getImaginary() {
        return imaginary;
    }

    public double abs() { //TODO normalize?
        return  Math.sqrt(real*real + imaginary*imaginary);
    }

    public double angleInDegree() {
        double angle = 360.0 * Math.atan2(imaginary, real) / (2*Math.PI);
        return angle < 0 ? angle + 360.0 : angle;
    }
}
