using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RainbowAnimation : MonoBehaviour
{
    private Vector2 uvOffset = Vector2.zero;
    private Vector2 animationRate = new Vector2(0.1f,0.0f);

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        uvOffset += animationRate * Time.deltaTime;

        if (GetComponent<Renderer>().enabled)
        {
            GetComponent<Renderer>().materials[0].SetTextureOffset("_MainTex", uvOffset);
        }
    }
}
