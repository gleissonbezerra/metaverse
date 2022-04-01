using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraMask : MonoBehaviour
{
    public Camera TargetCamera;
    public int LayerToRender;

    // Start is called before the first frame update
    void Start()
    {
        TargetCamera.transform.position = new Vector3(0,0,0);

        TargetCamera.cullingMask = 1 << LayerToRender;
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
