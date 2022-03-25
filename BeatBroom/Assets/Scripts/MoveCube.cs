using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MoveCube : MonoBehaviour
{
    private float movementSpeed = 10f;
    private bool hasCollision = false;

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        if ( !hasCollision )
        {
            transform.position = transform.position - new Vector3(0, 0, movementSpeed * Time.deltaTime);
        }
        
    }

    void OnCollisionEnter(Collision collision)
    {
        hasCollision = true;
    }
}
