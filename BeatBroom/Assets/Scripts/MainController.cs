using System.Collections;
using System.Collections.Generic;
using UnityEngine;


public class MainController : MonoBehaviour
{
    private int maxObjects = 5;
    private List<GameObject> currentObjects = new List<GameObject>();
    private float timerCreate;
    private float timerMove;

    public GameObject cubePrefab;

    // Start is called before the first frame update
    void Start()
    {
        timerCreate = Time.time;
        timerMove = Time.time;
    }

    // Update is called once per frame
    void Update()
    {
        if ( Time.time - timerCreate > 1)
        {
            timerCreate = Time.time;

            if ( currentObjects.Count < maxObjects)
            {
                currentObjects.Add( (GameObject)Instantiate(cubePrefab, new Vector3(Random.Range(-2.5F,2.5F), Random.Range(2.5F,3.5F), 50), Quaternion.identity));
            }else{
                Destroy(currentObjects[0]);
                currentObjects.RemoveAt(0);
            }

        }


        if ( Time.time - timerMove > 0.05)
        {
            timerMove = Time.time;

            foreach (GameObject obj in currentObjects)
            {
                
                if ( obj.transform.position.z < 0.5){
                    Destroy(obj);
                    currentObjects.Remove(obj);
                    break;
                }
            }
        }
        
    }
}
