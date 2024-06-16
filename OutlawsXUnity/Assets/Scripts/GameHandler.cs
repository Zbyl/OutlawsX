using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System.Reflection;

public class GameHandler : MonoBehaviour {

    public Material material;

    public static void Fill<T>(T[] array, T value)
    {
        for (int i = 0; i < array.Length; i++)
        {
            array[i] = value;
        }
    }

#if SUMMFIN // Use UNITY_EDITOR macro
    [DllImport("OutlawsI")]
    private static extern int func(int a);

    [DllImport("OutlawsI")]
    private static extern int loadLevel();

    [DllImport("OutlawsI")]
    private static extern unsafe void fetchData(Vector3* vertices, Vector2* uvs, int* numVertices, int* triangles, int* numTriangles);
    static int func(int a)
    {
        return Native.Invoke<int>(nativeLibraryPtr, (MethodInfo)MethodBase.GetCurrentMethod(), a);
    }

    static int loadLevel()
    {
        return Native.Invoke<int>(nativeLibraryPtr, (MethodInfo)MethodBase.GetCurrentMethod());
    }

    unsafe delegate void fetchDataDelegate(Vector3* vertices, Vector2* uvs, int* numVertices, int* triangles, int* numTriangles);
    unsafe static void fetchData(Vector3* vertices, Vector2* uvs, int* numVertices, int* triangles, int* numTriangles)
    {
        var del = Native.InvokeSimple<fetchDataDelegate>(nativeLibraryPtr, "fetchData");
        del(vertices, uvs, numVertices,triangles, numTriangles);
    }
#else

    struct TextureUvs
    {
        Vector4 texRect;
        int texWidth;
        int texHeight;
    }

    delegate IntPtr getErrorMessage();
    delegate IntPtr sectorFlag1ToString(int isector);
    delegate IntPtr wallFlag1ToString(int isector, int iwall);
    delegate IntPtr wallFlag2ToString(int isector, int iwall);

    delegate int func(int a);
    delegate int loadLevel(IntPtr levelFile, IntPtr textureFile);
    delegate int overlappingSector2d(Vector2 point);
    delegate int overlappingSector3d(Vector3 point);
    unsafe delegate void fetchTexInfos(TextureUvs* texInfos, int* numTexInfos);
    unsafe delegate void fetchData(Vector3* vertices, Vector4* uvs, int* numVertices, int* triangles, int* numTriangles, int sectorNum);
    delegate int getWallForTriangle(int isector, int triangleIndex);


    public static string getErrorMessage_()
    {
        var func = Native.InvokeSimple<getErrorMessage>(nativeLibraryPtr);
        var cstr = func();
        var str = (string)FreeCStrMarshaler.GetInstance("").MarshalNativeToManaged(cstr);
        FreeCStrMarshaler.GetInstance("").CleanUpNativeData(cstr);
        return str;
    }

    public static string sectorFlag1ToString_(int isector)
    {
        var func = Native.InvokeSimple<sectorFlag1ToString>(nativeLibraryPtr);
        var cstr = func(isector);
        var str = (string)FreeCStrMarshaler.GetInstance("").MarshalNativeToManaged(cstr);
        FreeCStrMarshaler.GetInstance("").CleanUpNativeData(cstr);
        return str;
    }

    public static string wallFlag1ToString_(int isector, int iwall)
    {
        var func = Native.InvokeSimple<wallFlag1ToString>(nativeLibraryPtr);
        var cstr = func(isector, iwall);
        var str = (string)FreeCStrMarshaler.GetInstance("").MarshalNativeToManaged(cstr);
        FreeCStrMarshaler.GetInstance("").CleanUpNativeData(cstr);
        return str;
    }

    public static string wallFlag2ToString_(int isector, int iwall)
    {
        var func = Native.InvokeSimple<wallFlag2ToString>(nativeLibraryPtr);
        var cstr = func(isector, iwall);
        var str = (string)FreeCStrMarshaler.GetInstance("").MarshalNativeToManaged(cstr);
        FreeCStrMarshaler.GetInstance("").CleanUpNativeData(cstr);
        return str;
    }

    static int func_(int a)
    {
        var val = Native.Invoke<int, func>(nativeLibraryPtr, a);
        var func = Native.InvokeSimple<func>(nativeLibraryPtr);
        return func(a);
    }

    public static int getWallForTriangle_(int isector, int triangleIndex)
    {
        var func = Native.InvokeSimple<getWallForTriangle>(nativeLibraryPtr);
        return func(isector, triangleIndex);
    }

    static int loadLevel_(string levelFile, string textureFile)
    {
        var func = Native.InvokeSimple<loadLevel>(nativeLibraryPtr);
        var levelFilePtr = FreeCStrMarshaler.GetInstance("").MarshalManagedToNative(levelFile);
        var textureFilePtr = FreeCStrMarshaler.GetInstance("").MarshalManagedToNative(textureFile);
        var result = func(levelFilePtr, textureFilePtr);
        FreeCStrMarshaler.GetInstance("").CleanUpNativeData(levelFilePtr);
        FreeCStrMarshaler.GetInstance("").CleanUpNativeData(textureFilePtr);
        if (result == -1)
        {
            var errorMessage = getErrorMessage_();
            Debug.LogError(string.Format("Error while loading level: {0}", errorMessage));
        }
        return result;
    }

    static int overlappingSector2d_(Vector2 point)
    {
        var func = Native.InvokeSimple<overlappingSector2d>(nativeLibraryPtr);
        return func(point);
    }

    static int overlappingSector3d_(Vector3 point)
    {
        var func = Native.InvokeSimple<overlappingSector3d>(nativeLibraryPtr);
        return func(point);
    }

    unsafe static void fetchTexInfos_(TextureUvs* texInfos, int* numTexInfos)
    {
        var func = Native.InvokeSimple<fetchTexInfos>(nativeLibraryPtr);
        func(texInfos, numTexInfos);
    }

    unsafe static void fetchData_(Vector3* vertices, Vector4* uvs, int* numVertices, int* triangles, int* numTriangles, int sectorNum)
    {
        var func = Native.InvokeSimple<fetchData>(nativeLibraryPtr);
        func(vertices, uvs, numVertices, triangles, numTriangles, sectorNum);
    }

    static IntPtr nativeLibraryPtr;

    public static GameHandler instance;

    void Awake()
    {
        GameHandler.instance = this;
        if (nativeLibraryPtr != IntPtr.Zero) return;

        nativeLibraryPtr = Native.LoadLibrary("OutlawsPlugin");
        if (nativeLibraryPtr == IntPtr.Zero)
        {
            Debug.LogError("Failed to load native library");
        }
    }

    void OnApplicationQuit()
    {
        if (nativeLibraryPtr == IntPtr.Zero) return;

        Debug.Log(Native.FreeLibrary(nativeLibraryPtr)
                      ? "Native library successfully unloaded."
                      : "Native library could not be unloaded.");
        nativeLibraryPtr = IntPtr.Zero;
    }
#endif

    public string levelFile = @"S:\VSProjects\OutlawsXDir\trash\HIDEOUT.LVT"; // Can be LVT or LVB.
    public string texturesPackFile = @"S:\VSProjects\OutlawsXDir\OutlawsX\OutlawsXUnity\Assets\Textures\pack.json"; 
    public Transform marker;
    private Dictionary<int, GameObject> sectorObjects = new Dictionary<int, GameObject>();
    private Dictionary<int, Mesh> meshes = new Dictionary<int, Mesh>();
    public int sectorNum = -1;
    public int lastSectorNum = -1;

    private ComputeBuffer texInfos;

    // Use this for initialization
    unsafe void Start()
    {
        var b = func_(4);
        Debug.Log(string.Format("Hello: {0}", b));

        var numSectors = loadLevel_(levelFile, texturesPackFile);
        Debug.Log(string.Format("numSectors: {0}", numSectors));

        var texInfosArray = new TextureUvs[1000];
        int numTexInfos = texInfosArray.Length;
        fixed (TextureUvs* texInfosPtr = texInfosArray)
        {
            fetchTexInfos_(texInfosPtr, &numTexInfos);
        }
        Debug.Log(string.Format("numTexInfos: {0}", numTexInfos));
        if (numTexInfos == 0)
            Debug.LogErrorFormat("Error loading texInfos.");

        texInfos = new ComputeBuffer(numTexInfos, 6 * 4);
        texInfos.SetData(texInfosArray, 0, 0, numTexInfos);
        material.SetBuffer("texInfos", texInfos);

        for (int i = 0; i < numSectors; ++i)
        {
            prepareSectorObject(i);
            updateMesh(i);
        }
    }

    void prepareSectorObject(int sectorNum)
    {
        GameObject sectorObject = new GameObject(string.Format("Sector {0}", sectorNum), typeof(MeshFilter), typeof(MeshRenderer), typeof(MeshCollider), typeof(Sector));
        //sectorObject.transform.localScale = new Vector3(0.1f, 0.1f, 0.1f);
        sectorObjects.Add(sectorNum, sectorObject);

        // Dynamic meshes: https://gamedev.stackexchange.com/a/136170
        Mesh mesh = new Mesh();
        mesh.MarkDynamic();

        sectorObject.GetComponent<Sector>().isector = sectorNum;
        sectorObject.GetComponent<MeshFilter>().mesh = mesh;
        sectorObject.GetComponent<MeshRenderer>().material = material;
        sectorObject.GetComponent<MeshCollider>().sharedMesh = mesh;

        Debug.LogFormat("Mesh is readable: {0}", mesh.isReadable);

        meshes.Add(sectorNum, mesh);
    }

    unsafe void updateMesh(int sectorNum) {
        Vector3[] vertices = new Vector3[2500];
        Vector4[] uvs = new Vector4[2500];
        int[] triangles = new int[3600];

        //Fill(vertices, Vector3.zero);
        //Fill(uvs, Vector2.zero);
        //Fill(triangles, 0);

        int numVertices = vertices.Length;
        int numTriangles = triangles.Length;

        fixed (Vector3* verts = vertices)
        {
            fixed (Vector4* texCoords = uvs)
            {
                fixed (int* tris = triangles)
                {
                    fetchData_(verts, texCoords, & numVertices, tris, &numTriangles, sectorNum);
                }
            }
        }

        //Debug.Log(string.Format("sectorNum={0} numVertices={1} numTriangles={2}", sectorNum, numVertices, numTriangles));
        if (numVertices == 0)
            Debug.LogErrorFormat("Error loading sectorNum={0} ", sectorNum);

        var verts2 = new Vector3[numVertices];
        for (int i = 0; i < numVertices; ++i)
            verts2[i] = vertices[i];

        var uvs2 = new Vector4[numVertices];
        for (int i = 0; i < numVertices; ++i)
            uvs2[i] = uvs[i];

        var tris2 = new int[numTriangles];
        for (int i = 0; i < numTriangles; ++i)
            tris2[i] = triangles[i];

        Mesh mesh = meshes[sectorNum];
        mesh.vertices = verts2;
        var uvsList = new List<Vector4>(uvs2);
        mesh.SetUVs(0, uvsList);
        mesh.triangles = tris2;

        mesh.RecalculateNormals();
        // mesh.tangents = ... - set tangents after recalculating normals.
        mesh.RecalculateBounds();

        var sectorObject = sectorObjects[sectorNum];
        sectorObject.GetComponent<MeshCollider>().sharedMesh = null;
        sectorObject.GetComponent<MeshCollider>().sharedMesh = mesh;
    }

    void ala() {
        Vector3[] vertices = new Vector3[4] {
            new Vector3(0, 1),
            new Vector3(1, 1),
            new Vector3(0, 0),
            new Vector3(1, 0),
        };
        Vector2[] uvs = new Vector2[4] {
            new Vector2(0, 1),
            new Vector2(1, 1),
            new Vector2(0, 0),
            new Vector2(1, 0),
        };
        int[] triangles = new int[2 * 3] {
            0, 1, 2,
            2, 1, 3,
        };

        // Dynamic meshes: https://gamedev.stackexchange.com/a/136170
        Mesh mesh = new Mesh();
        //MeshUtility.SetMeshCompression(myMesh, ModelImporterMeshCompression.High);
        mesh.MarkDynamic();

        mesh.vertices = vertices;
        mesh.uv = uvs;
        mesh.triangles = triangles;

        GameObject gameObject = new GameObject("MyMesh", typeof(MeshFilter), typeof(MeshRenderer), typeof(MeshCollider));
        gameObject.transform.localScale = new Vector3(30, 30, 1);

        mesh.RecalculateNormals();
        // mesh.tangents = ... - set tangents after recalculating normals.

        mesh.RecalculateBounds();

        gameObject.GetComponent<MeshFilter>().mesh = mesh;

        gameObject.GetComponent<MeshRenderer>().material = material;

        gameObject.GetComponent<MeshCollider>().sharedMesh = mesh;
    }

    // Update is called once per frame
    void Update () {
        if (sectorNum != lastSectorNum)
        {
            float x = marker.position.x * 10.0f;
            float z = marker.position.z * 10.0f;
            int currentSector = overlappingSector2d_(new Vector2(x, z));
            Debug.Log(string.Format("currentSector={0}", currentSector));
            material.SetBuffer("texInfos", texInfos);

            lastSectorNum = sectorNum;
            if (!sectorObjects.ContainsKey(sectorNum))
            {
                prepareSectorObject(sectorNum);
                updateMesh(sectorNum);
            }
        }
    }

}
