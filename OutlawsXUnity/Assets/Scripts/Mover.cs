using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Mover : MonoBehaviour {

    // Use this for initialization
    void Start () {
        
    }
    
    // Update is called once per frame
    void Update () {
        
    }

    float rX = 0.0f;
    float rY = 0.0f;
    public float rotSpeed = 1.5f;
    public float moveSpeed = 0.5f;
    public float mouseScale = 10.0f;
    public float upDownSpeed = 0.5f;

    void FixedUpdate()
    {
        float moveH = Input.GetAxis("Horizontal") * moveSpeed;
        float moveV = Input.GetAxis("Vertical") * moveSpeed;
        float moveUpDown = Input.GetAxis("UpDown") * upDownSpeed;
        float rotX = (Input.GetAxis("RotX") + Input.GetAxis("Mouse X") * mouseScale) * rotSpeed;
        float rotY = (Input.GetAxis("RotY") + Input.GetAxis("Mouse Y") * mouseScale) * rotSpeed;
        rX += rotX;
        rY += rotY;

        transform.position += moveV * transform.forward;
        transform.position += moveH * transform.right;
        transform.position += moveUpDown * transform.up;

        transform.rotation = Quaternion.AngleAxis(rX, Vector3.up) * Quaternion.AngleAxis(-rY, Vector3.right);

        if (Input.GetButton("Fire1"))
        {
            RaycastHit hit;

            if (Physics.Raycast(transform.position, transform.forward, out hit))
            {
                var sector = hit.transform.GetComponent<Sector>();
                var isector = (sector != null) ? sector.isector : -2;
                var iwall = 0;
                string sectorFlag1 = "<no sector>";
                string wallFlag1 = "<no wall>";
                string wallFlag2 = "<no wall>";
                if (sector != null)
                {
                    sectorFlag1 = GameHandler.sectorFlag1ToString_(isector);
                    iwall = GameHandler.getWallForTriangle_(isector, hit.triangleIndex);
                    if (iwall == -1)
                    {
                        wallFlag1 = "<floor>";
                        wallFlag2 = "<floor>";
                    }
                    else
                    if (iwall == -2)
                    {
                        wallFlag1 = "<ceiling>";
                        wallFlag2 = "<ceiling>";
                    }
                    else
                    {
                        wallFlag1 = GameHandler.wallFlag1ToString_(isector, iwall);
                        wallFlag2 = GameHandler.wallFlag2ToString_(isector, iwall);
                    }
                }
                Debug.LogFormat("Hit: {0} distance={1} triangle={2} sector={3} f1={4} wall={5} f1={6} f2={7}", hit.transform.gameObject.name, hit.distance, hit.triangleIndex, isector, sectorFlag1, iwall, wallFlag1, wallFlag2);
                //print("Found an object - distance: " + hit.distance);
            }
        }
    }
}
