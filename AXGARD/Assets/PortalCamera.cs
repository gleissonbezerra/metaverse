using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PortalCamera : MonoBehaviour
{

    public Transform playerCamera;
    public Transform playerPortal; 
    public Transform referencePortal;  

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        Vector3 playerOffset = playerCamera.position - referencePortal.position;
        transform.position = playerPortal.position + playerOffset; 

        float angleDiff = Quaternion.Angle(playerPortal.rotation, referencePortal.rotation);

        Quaternion portalRotationDiff = Quaternion.AngleAxis( angleDiff, Vector3.up);
        Vector3 newCameraOrientation = portalRotationDiff * playerCamera.forward;

        transform.rotation = Quaternion.LookRotation( newCameraOrientation, Vector3.up);

    }
}
