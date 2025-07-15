package com.brazedblue.intervox.data;

import com.brazedblue.intervox.geometry.*;
import com.brazedblue.intervox.util.Debug;
import java.util.*;

public class CalculatedOffset extends DataModel
    implements java.io.Serializable, java.lang.Cloneable {
  FloatPoint fOffset = new FloatPoint();
  FloatPoint fStandardDeviation = new FloatPoint();
  ThreeDVector fOrthogonalVector;
  String fLabel;

  Matrix3 fRotation; // rotation of vector [1,0,0] to normalized offset

  transient Vector<FloatPoint> fPoints; // recorded points
  transient Vector<Matrix3> fAngles; // recorded angles

  private static final int kDecimalPlaces = 2;

  static final long serialVersionUID = 33805524761736785L;

  public CalculatedOffset(String label) {
    fLabel = label;
  }

  public CalculatedOffset() {
    this("");
  }

  public synchronized Object clone() {
    CalculatedOffset result = (CalculatedOffset) super.clone();

    result.fOffset = new FloatPoint(fOffset);
    result.fStandardDeviation = new FloatPoint(fStandardDeviation);
    if (fOrthogonalVector != null) {
      result.fOrthogonalVector = new ThreeDVector((Location3D) fOrthogonalVector);
    }

    if (fPoints != null) {
      result.fPoints = (Vector<FloatPoint>) fPoints.clone();
    }

    if (fAngles != null) {
      result.fAngles = (Vector<Matrix3>) fAngles.clone();
    }

    if (fRotation != null) {
      result.fRotation = new Matrix3(fRotation);
    }

    return result;
  }

  private void CreateVectors() {
    if (fPoints == null) {
      fPoints = new Vector<FloatPoint>();
      fAngles = new Vector<Matrix3>();
    }
  }

  private void ComputeRotationIfNecessary() {
    if (fRotation == null) {
      fRotation = new Matrix3();
      fRotation.setIdentity();

      if (!fOffset.equals(FloatPoint.kZero)) {
        ThreeDVector vector = new ThreeDVector((Location3D) fOffset);
        vector.normalize();

        double angle = Math.acos(vector.GetFloat(Location3D.X_AXIS));

        if (vector.GetFloat(Location3D.Y_AXIS) < 0) {
          angle = 2 * Math.PI - angle;
        }

        fRotation.rotZ((float) angle);

        fRotation.transform(vector);

        angle = Math.acos(vector.GetFloat(Location3D.X_AXIS));

        fRotation.rotY((float) angle);
      }
    }
  }

  public int GetPointCount() {
    CreateVectors();
    return fPoints.size();
  }

  public void AddPointAndAngle(FloatPoint point, Matrix3 rotation) {
    CreateVectors();
    fPoints.addElement(point);
    fAngles.addElement(rotation);
  }

  public void ClearPointsAndAngles() {
    CreateVectors();
    fPoints.removeAllElements();
    fAngles.removeAllElements();
  }

  public Vector<FloatPoint> GetPoints() {
    CreateVectors();
    return fPoints;
  }

  public Matrix3 GetRotation() {
    ComputeRotationIfNecessary();

    return fRotation;
  }

  public Location3D GetOrthogonalVector() {
    if (fOrthogonalVector == null) {
      fOrthogonalVector = new ThreeDVector();
      ComputeOrthogonalVector();
    }

    return fOrthogonalVector;
  }

  private void ComputeOrthogonalVector() {

    if (!fOffset.Equals(FloatPoint.kZero)) {
      ThreeDVector normalizedOffset = new ThreeDVector((Location3D) fOffset);
      normalizedOffset.normalize();

      // Matrix3 rotater = new Matrix3();

      // this computes an orthogonal vector in the XY plane

      float offsetX = normalizedOffset.GetFloat(Location3D.X_AXIS);
      float offsetY = normalizedOffset.GetFloat(Location3D.Y_AXIS);

      if (offsetX == 0 && offsetY == 0) {
        fOrthogonalVector.set(0, 1, 0); // any vector in XY plane, offet is on z axis
      } else if (offsetX * offsetY < 0) {
        fOrthogonalVector.set((float) Math.abs(offsetY), (float) Math.abs(offsetX), 0);
        fOrthogonalVector.normalize();
      } else {
        fOrthogonalVector.set(offsetY, -offsetX, 0);
        fOrthogonalVector.normalize();
      }
      if (Debug.fgDebugging && false) {
        System.out.println("normalizedOffset: " + normalizedOffset.toString());
        System.out.println("fOrthogonalVector: " + fOrthogonalVector.toString());

        System.out.print("Angle ");
        float angle = normalizedOffset.angle(fOrthogonalVector);
        Debug.PrintRadAngleAsDegree(angle, System.out);
        System.out.println("");
        System.out.println("");
      }
      /*
       * if (normalizedOffset.GetFloat(Location3D.X_AXIS) != 0)
       * {
       * rotater.rotZ((float)Math.asin(normalizedOffset.GetFloat(Location3D.Y_AXIS) /
       * ));
       * rotater.transform(fOrthogonalVector);
       * }
       * rotater.setIdentity();
       * rotater.rotZ((float)Math.asin(normalizedOffset.GetFloat(Location3D.Y_AXIS)));
       * rotater.transform(fOrthogonalVector);
       *
       * if (neurosynch.util.Debug.fgDebugging)
       * {
       * System.out.print("fOrthogonalVector after z axis angle ");
       * float angle = (float)Math.asin(normalizedOffset.GetFloat(Location3D.Y_AXIS));
       * neurosynch.util.Debug.PrintRadAngleAsDegree(angle, System.out);
       * System.out.println(fOrthogonalVector.toString());
       * System.out.print("Angle ");
       * angle = normalizedOffset.angle(new
       * ThreeDVector((Location3D)fOrthogonalVector));
       * neurosynch.util.Debug.PrintRadAngleAsDegree(angle, System.out);
       * System.out.println("");
       * }
       *
       * rotater.setIdentity();
       * rotater.rotY((float)Math.asin(normalizedOffset.GetFloat(Location3D.Z_AXIS)));
       * rotater.transform(fOrthogonalVector);
       *
       * if (neurosynch.util.Debug.fgDebugging)
       * {
       * System.out.print("fOrthogonalVector after Y axis angle ");
       * float angle = (float)Math.asin(normalizedOffset.GetFloat(Location3D.Z_AXIS));
       * neurosynch.util.Debug.PrintRadAngleAsDegree(angle, System.out);
       * System.out.println("");
       *
       * System.out.println("normalizedOffset: " + normalizedOffset.toString());
       * System.out.println("fOrthogonalVector: " + fOrthogonalVector.toString());
       *
       * System.out.print("Angle ");
       * angle = normalizedOffset.angle(new
       * ThreeDVector((Location3D)fOrthogonalVector));
       * neurosynch.util.Debug.PrintRadAngleAsDegree(angle, System.out);
       * System.out.println("");
       * System.out.println("");
       * }
       */
    }
  }

  public Vector GetAngles() {
    CreateVectors();
    return fAngles;
  }

  public void SetOffset(FloatPoint point) {
    fOffset.Set(point);
    fOrthogonalVector = null; // must recalculate
    PostChanged(new DataChangedEvent(fOffset, DataChangedEvent.CHANGED, 0, null));
  }

  public FloatPoint GetOffset() {
    return fOffset;
  }

  public void SetDeviation(FloatPoint point) {
    fStandardDeviation.Set(point);
    // shouldn't need to post changed for this, always happens with SetOffset
  }

  public String toString() {
    return fLabel
        + ": "
        + fOffset.ToString(kDecimalPlaces)
        + " err: "
        + fStandardDeviation.ToString(kDecimalPlaces);
  }

  public void SetLabel(String label) {
    fLabel = label;
    PostChanged(new DataChangedEvent(fLabel, DataChangedEvent.CHANGED, 0, null));
  }

  public String GetLabel() {
    return fLabel;
  }
}
