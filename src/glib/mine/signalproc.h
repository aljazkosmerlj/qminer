#ifndef SIGNALPROC_H_
#define SIGNALPROC_H_

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "avltree.h"

namespace TSignalProc {

/////////////////////////////////////////////////
// Online Moving Average
class TMaSimple {
private:
	TFlt Ma; // current computed MA value  
	TUInt64 N;
public:
	TMaSimple() { };
	TMaSimple(const PJsonVal& ParamVal) { };
	TMaSimple(TSIn& SIn) : Ma(SIn), N(SIn) {}
	// serialization
	void Load(TSIn& SIn) { *this = TMaSimple(SIn); }
	void Save(TSOut& SOut) const { Ma.Save(SOut); N.Save(SOut); }
	void Update(const double& InVal) { Ma = Ma + (InVal - Ma) / ++N; }
	double GetMa() const { return Ma; }
	void Clr() { Ma = 0; N = 0; }
};

/////////////////////////////////////////////////
// Online M2 (variance)
class TVarSimple {
private:
	TFlt OldM;
	TFlt NewM;
	TFlt OldS;
	TFlt NewS;
	TUInt64 N;
public:
	TVarSimple() { }
	TVarSimple(TSIn& SIn): OldM(SIn), NewM(SIn), OldS(SIn), NewS(SIn), N(SIn) {}

	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;
	void Update(const double& InVal);
	// current status	
	uint64 GetN() const { return N; }
	double GetMean() const { return N > 0 ? (double)NewM : 0.0; }
	double GetStDev() const { return sqrt((double)GetVar()); }
	double GetVar() const { return (N > 1 ? NewS / (N - 1) : 0.0); }
	void Clr() { OldM = NewM = OldS = NewS = 0.0; N = 0; }
};

/////////////////////////////////////////////////
// Online Moving Average
class TMa {
private:	
    TFlt Ma; // current computed MA value 
	TUInt64 TmMSecs; // timestamp of current MA	    
public:
	TMa() { };
    TMa(const PJsonVal& ParamVal) { };
	TMa(TSIn& SIn);
	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;

    bool IsInit() const { return (TmMSecs > 0); }
	/// Resets the model state
	void Reset() { Ma = 0.0; TmMSecs = 0; }
	void Update(const double& InVal, const uint64& InTmMSecs,
        const TFltV& OutValV, const TUInt64V& OutTmMSecs, const int& N);	
	void Update(const TFltV& InValV, const TUInt64V& InTmMSecsV, const TFltV& OutValV, const TUInt64V& OutTmMSecs, const int& N) { throw  TExcept::New("TSignalProc::TMa, delayed Update not implemented"); }
	double GetValue() const { return Ma; }
	uint64 GetTmMSecs() const { return TmMSecs; }
	void Clr() { Ma = 0; TmMSecs = 0; }
};
    
/////////////////////////////////////////////////
// Online Summa
class TSum {
private:
	TFlt Sum; // current computed SUM value 
	TUInt64 TmMSecs; // timestamp of current MA	    
public:
	TSum() { };
	TSum(const PJsonVal& ParamVal) { };
	TSum(TSIn& SIn): Sum(SIn), TmMSecs(SIn) { }

	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;

    bool IsInit() const { return (TmMSecs > 0); }
	/// Resets the model state
	void Reset() { Sum = 0; TmMSecs = 0; }
	void Update(const double& InVal, const uint64& InTmMSecs,
		const TFltV& OutValV, const TUInt64V& OutTmMSecs);
	void Update(const TFltV& InValV, const TUInt64V& InTmMSecsV, const TFltV& OutValV, const TUInt64V& OutTmMSecs) { throw  TExcept::New("TSignalProc::TSum, delayed Update not implemented"); }

	double GetValue() const { return Sum; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
/// Online Summa of sparse vectors
class TSumSpVec {
private:
	TVec<TIntFltKd> Sum; ///< current computed SUM value 
	TUInt64 TmMSecs; ///< timestamp of current MA	    
public:
	TSumSpVec() {}; ///< Simple constructor
	TSumSpVec(const PJsonVal& ParamVal) {}; ///< Initialization from JSON value
	TSumSpVec(TSIn& SIn) : Sum(SIn), TmMSecs(SIn) {} ///> Deserialization constructor

	// serialization
	void Load(TSIn& SIn); ///< Loading from binary stream
	void Save(TSOut& SOut) const; ///< Saving to binary stream

	bool IsInit() const { return (TmMSecs > 0); } ///< Checks if this sum received any data yet
	/// Resets the model state
	void Reset() { Sum = TIntFltKdV(); TmMSecs = 0; }
	/// Updates internal state with incoming and outgoing data
	void Update(const TVec<TIntFltKd>& InVal, const uint64& InTmMSecs, const TVec<TIntFltKdV>& OutValV, const TUInt64V& OutTmMSecs);
	/// Updates internal state with incoming and outgoing data - for delayed update
	void Update(const TVec<TIntFltKdV>& InValV, const TUInt64V& InTmMSecsV, const TVec<TIntFltKdV>& OutValV, const TUInt64V& OutTmMSecs) {
		throw  TExcept::New("TSignalProc::TSumSpVec, delayed Update not implemented"); }

	const TIntFltKdV& GetValue() const { return Sum; } ///< Access current sum
	uint64 GetTmMSecs() const { return TmMSecs; } ///< Access last received timestampe
	PJsonVal GetJson() const; ///< Get JSON description of the sum
};
/////////////////////////////////////////////////
// Sliding Window Min
class TMin {
private:
	TFlt Min; // current computed SUM value 
	TUInt64 TmMSecs; // timestamp of current MA	   
	TFltUInt64PrV AllValV; // sorted vector of values	
public:
	TMin(): Min(TFlt::Mx) { };
	TMin(TSIn& SIn): Min(SIn), TmMSecs(SIn), AllValV(SIn) { }

	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;

    bool IsInit() const { return (TmMSecs > 0); }
	/// Resets the model state
	void Reset() { Min = TFlt::Mx; TmMSecs = 0; }
	void Update(const double& InVal, const uint64& InTmMSecs,
		const TFltV& OutValV, const TUInt64V& OutTmMSecs);
	void Update(const TFltV& InValV, const TUInt64V& InTmMSecsV, const TFltV& OutValV, const TUInt64V& OutTmMSecs) { throw  TExcept::New("TSignalProc::TMin, delayed Update not implemented"); }
	double GetValue() const { return Min; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
// Sliding Window Max
class TMax {
private:
	TFlt Max; // current computed SUM value 
	TUInt64 TmMSecs; // timestamp of current MA	   
	TFltUInt64PrV AllValV; // sorted vector of values	
public:
	TMax(): Max(TFlt::Mn) { };
	TMax(TSIn& SIn): Max(SIn), TmMSecs(SIn), AllValV(SIn) { }

	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;

    bool IsInit() const { return (TmMSecs > 0); }
	/// Resets the model state
	void Reset() { Max = TFlt::Mn; TmMSecs = 0; }
	void Update(const double& InVal, const uint64& InTmMSecs,
		const TFltV& OutValV, const TUInt64V& OutTmMSecs);
	void Update(const TFltV& InValV, const TUInt64V& InTmMSecsV, const TFltV& OutValV, const TUInt64V& OutTmMSecs) { throw  TExcept::New("TSignalProc::TMax, delayed Update not implemented"); }
	double GetValue() const { return Max; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
// Exponential Moving Average
typedef enum { etPreviousPoint, etLinear, etNextPoint } TEmaType;
class TEma {
private:
	// parameters
	TFlt Decay; ///< decaying factor
	TEmaType Type; ///< interpolation type
	// current state
	TFlt LastVal; ///< last input value
	TFlt Ema; ///< current computed EMA value 
	TUInt64 TmMSecs; ///< timestamp of current EMA
	double TmInterval; ///< time interval for definition of decay
	// buffer for initialization
	TBool InitP; ///< true if already initialized
	TUInt64 InitMinMSecs; ///< time window of required values for initialization
	TFltV InitValV; ///< first N values
	TUInt64V InitMSecsV; ///< weights of first N values
 	
	double GetNi(const double& Alpha, const double& Mi);
public:
	TEma(const double& _Decay, const TEmaType& _Type, 
        const uint64& _InitMinMSecs, const double& _TmInterval);
	TEma(const TEmaType& _Type, const uint64& _InitMinMSecs,
        const double& _TmInterval);
    TEma(const PJsonVal& ParamVal);
	TEma(TSIn& SIn);
	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;

	void Update(const double& Val, const uint64& NewTmMSecs);
	// current status
	bool IsInit() const { return InitP; }
	/// Resets the aggregate
	void Reset();
	double GetValue() const { return Ema; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};


class TEmaSpVec {
private:
	// parameters
	TEmaType Type; ///< interpolation type
    // current state
	TIntFltKdV LastVal; ///< last input value
	TIntFltKdV Ema; ///< current computed EMA value 
	TUInt64 TmMSecs; ///< timestamp of current EMA
	TFlt TmInterval; ///< time interval for definition of decay
	TFlt Cutoff; ///< Minimal value for dimension - if it falls below this, it is removed from Ema
	// buffer for initialization
	TBool InitP; ///< true if already initialized
	TUInt64 InitMinMSecs; ///< time window of required values for initialization
	TVec<TIntFltKdV> InitValV; ///< first N values
	TUInt64V InitMSecsV; ///< weights of first N values

	double GetNi(const double& Alpha, const double& Mi);
public:
	TEmaSpVec(const TEmaType& _Type, const uint64& _InitMinMSecs, const double& _TmInterval, const double& _Cutoff);
	TEmaSpVec(const PJsonVal& ParamVal);
	TEmaSpVec(TSIn& SIn);

	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;

	void Update(const TIntFltKdV& Val, const uint64& NewTmMSecs);
	// current status
	bool IsInit() const { return InitP; }
	
	/// Resets the aggregate
	void Reset();
	const TIntFltKdV GetValue() const { return Ema; }
	uint64 GetTmMSecs() const { return TmMSecs; }
	
	PJsonVal GetJson() const; ///< Get JSON description of the sum
};
/////////////////////////////////////////////////
// Online M2 (variance)
class TVar {
private:	
    TFlt Ma; // current computed MA value 
    TFlt M2; // current computed M2 value 
	TUInt64 TmMSecs; // timestamp of current WMA	
    TFlt pNo;
public:
	TVar(): pNo(1) { }
	TVar(TSIn& SIn): Ma(SIn), M2(SIn), TmMSecs(SIn), pNo(SIn) { }

	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;

    bool IsInit() const { return (TmMSecs > 0); }
	/// Resets the model state
	void Reset() { Ma = 0.0; M2 = 0.0; TmMSecs = 0; pNo = 1; }
	void Update(const double& InVal, const uint64& InTmMSecs,
        const TFltV& OutValV, const TUInt64V& OutTmMSecsV, const int& N);
	void Update(const TFltV& InValV, const TUInt64V& InTmMSecsV, const TFltV& OutValV, const TUInt64V& OutTmMSecs, const int& N) { throw  TExcept::New("TSignalProc::TVar, delayed Update not implemented"); }

	// current status	
	double GetValue() const { return (pNo > 1) ? (M2 / (pNo - 1)) : 0; }
	uint64 GetTmMSecs() const { return TmMSecs; }
	void Clr() { Ma = 0; M2 = 0; pNo = 1; TmMSecs = 0; }
};

/////////////////////////////////////////////////
// Online M2(X,Y) (covariance)
class TCov {
private:	
	TFlt MaX, MaY; // current computed MA value        
	TFlt Cov;
	TUInt64 TmMSecs; // timestamp of current WMA	
	TFlt pNo;
public:
	TCov() { };
    TCov(const PJsonVal& ParamVal) { };
	/// Resets the aggregates
	void Reset() { TmMSecs = 0; Cov = 0.0; pNo = 0.0; MaX = 0.0; MaY = 0.0; }
	void Update(const double& InValX, const double& InValY, const uint64& InTmMSecs, 
        const TFltV& OutValVX, const TFltV& OutValVY, const TUInt64V& OutTmMSecsV, const int& N);	
	double GetCov() const { return (pNo > 1) ? (Cov / (pNo - 1)) : 0; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};

///////////////////////////////
// Sequence buffer
template <class TVal>
class TBuffer {
private:
    // buffer size
	TInt BufferLen;
    // next place in buffer
	TInt NextValN;
    // internal vector used to store circular buffer
	TVec<TVal> ValV;
    
public:
	TBuffer(const int& BufferLen_): BufferLen(BufferLen_), 
        NextValN(0), ValV(BufferLen_, 0) { }

	// serialization
	void Load(TSIn& SIn) {
		BufferLen.Load(SIn);
		NextValN.Load(SIn);
		ValV.Load(SIn);
	}
	void Save(TSOut& SOut) const {
		BufferLen.Save(SOut);
		NextValN.Save(SOut);
		ValV.Save(SOut);
	}
    
    /// add new value to the buffer
    void Update(const TVal& Val){
        // we add to the end while not full
        if (ValV.Len() < BufferLen) { ValV.Add(Val); }
        // after we overwrite existing
        else { ValV[NextValN] = Val; }
        // we always point to next empty(-to-be) place
        NextValN++;
        // reset count once we come to BufferLen
        NextValN = NextValN % BufferLen;
    }
    
    /// Is buffered initialized 
    bool IsInit() const { return ValV.Len() == BufferLen; }
	/// Resets the buffer
	void Reset() { NextValN = 0; ValV.Gen(0); }
    /// Is buffer empty
    bool Empty() const { return ValV.Empty(); }
    /// Number of elements at the moment
    int Len() const { return ValV.Len(); }
        
	/// Get values from buffer (older values = larger index):
    ///  example buffer = [x_(t-3) x_(t-2) x_(t-1) x_t] => GetVal(i) returns x_(t-1)
	const TVal& GetVal(const int& ValN) const {
		return ValV[(NextValN + BufferLen - 1 - ValN) % BufferLen]; }
    /// Get the oldest value
	const TVal& GetOldest() const {
        Assert(!Empty()); return IsInit() ? GetVal(BufferLen - 1) : ValV[0]; }
	/// Get the most recent value
	const TVal& GetNewest() const {
        Assert(!Empty()); return IsInit() ?  GetVal(0) : ValV.Last(); }
};

///////////////////////////////
// Linked buffer
template <class TVal, class TSizeTy = TUInt64>
class TLinkedBuffer {
private:
	class Node {
	public:
		Node* Next;
		const TVal Val;

		Node(Node* _Next, const TVal& _Val): Next(_Next), Val(_Val) {}
	};

private:
	Node* First;
	Node* Last;
	TSizeTy Size;

public:
	TLinkedBuffer();
	TLinkedBuffer(TSIn& SIn);

	void Save(TSOut& SOut) const;

	~TLinkedBuffer();

	void Add(const TVal& Val);
	void DelOldest();
	void Clr();

	const TVal& GetOldest(const TSizeTy& Idx) const;
	const TVal& GetOldest() const { return GetOldest(0); };
	const TVal& GetNewest() const;

	bool Empty() const { return Len() == 0; };
	TSizeTy Len() const { return Size; };
};

template <class TVal, class TSizeTy>
TLinkedBuffer<TVal, TSizeTy>::TLinkedBuffer():
		First(NULL),
		Last(NULL),
		Size() {}

template <class TVal, class TSizeTy>
TLinkedBuffer<TVal, TSizeTy>::TLinkedBuffer(TSIn& SIn):
		First(NULL),
		Last(NULL),
		Size(SIn) {

	if (Size > 0) { First = new TLinkedBuffer<TVal, TSizeTy>::Node(NULL, TVal(SIn)); }

	Node* Curr = First;
	for (TSizeTy i = 1; i < Size; i++) {
		Curr->Next = new Node(NULL, TVal(SIn));
		Curr = Curr->Next;
	}

	Last = Curr;
}

template <class TVal, class TSizeTy>
void TLinkedBuffer<TVal, TSizeTy>::Save(TSOut& SOut) const {
	Size.Save(SOut);

	Node* Curr = First;
	while (Curr != NULL) {
		Curr->Val.Save(SOut);
		Curr = Curr->Next;
	}
}

template <class TVal, class TSizeTy>
TLinkedBuffer<TVal, TSizeTy>::~TLinkedBuffer() {
	Clr();
}

template <class TVal, class TSizeTy>
void TLinkedBuffer<TVal, TSizeTy>::Add(const TVal& Val) {
	TLinkedBuffer<TVal, TSizeTy>::Node* Node = new TLinkedBuffer<TVal, TSizeTy>::Node(NULL, Val);

	if (Size++ == 0) {
		First = Node;
		Last = Node;
	} else {
		Last->Next = Node;
		Last = Node;
	}
}

template <class TVal, class TSizeTy>
void TLinkedBuffer<TVal, TSizeTy>::DelOldest() {
	IAssertR(!Empty(), "Cannot delete elements from empty buffer!");

	Node* Temp = First;

	if (--Size == 0) {
		First = NULL;
		Last = NULL;
	} else {
		First = First->Next;
	}

	delete Temp;
}

template <class TVal, class TSizeTy>
void TLinkedBuffer<TVal, TSizeTy>::Clr() {
	while (!Empty()) { DelOldest(); }
}

template <class TVal, class TSizeTy>
const TVal& TLinkedBuffer<TVal, TSizeTy>::GetOldest(const TSizeTy& Idx) const {
	IAssertR(Idx < Size, "Index of element greater then size!");

	Node* Curr = First;
	for (TSizeTy i = 0; i < Idx; i++) {
		Curr = Curr->Next;
	}

	return Curr->Val;
}

template <class TVal, class TSizeTy>
const TVal& TLinkedBuffer<TVal, TSizeTy>::GetNewest() const {
	IAssertR(!Empty(), "Cannot return elements from empty buffer!");
	return Last->Val;
}

/////////////////////////////////////////
// Time series interpolator interface
class TInterpolator;
typedef TPt<TInterpolator> PInterpolator;
class TInterpolator {
private:
    TCRef CRef;
    friend class TPt<TInterpolator>;
private:
    const TStr InterpolatorType;
protected:
    TInterpolator(const TStr& _InterpolatorType): InterpolatorType(_InterpolatorType) {}
    TInterpolator(TSIn& SIn): InterpolatorType(SIn) {}

public:
	static PInterpolator New(const TStr& InterpolatorType);
	static PInterpolator Load(TSIn& SIn);

 	virtual ~TInterpolator() { }

	virtual void Save(TSOut& SOut) const { InterpolatorType.Save(SOut); }

	virtual void SetNextInterpTm(const uint64& Time) = 0;
	virtual double Interpolate(const uint64& Time) const = 0;
	virtual bool CanInterpolate(const uint64& Time) const = 0;
	virtual void AddPoint(const double& Val, const uint64& Tm) = 0;
};

/////////////////////////////////////////
// Buffered interpolator
// contains a buffer
// the first timestamp in the buffer is less or equal to the current time
// the other timestamps in the buffer are greater then the current time
class TBufferedInterpolator: public TInterpolator {
protected:
	// buffer holding the current and future points
	TLinkedBuffer<TPair<TUInt64, TFlt>> Buff;

	TBufferedInterpolator(const TStr& InterpolatorType);
	TBufferedInterpolator(const TStr& InterpolatorType, TSIn& SIn);

public:
	virtual void Save(TSOut& SOut) const;

	void AddPoint(const double& Val, const uint64& Tm);
};

/////////////////////////////////////////
// Previous point interpolator.
// Interpolate by returning previously seen value
// this interpolator will wait until it gets one value in the future before
// performing interpolation
class TPreviousPoint : public TBufferedInterpolator {
private:
	TPreviousPoint();
	TPreviousPoint(TSIn& SIn);
public:	
    static PInterpolator New() { return new TPreviousPoint; }
    static PInterpolator New(TSIn& SIn) { return new TPreviousPoint(SIn); }
    
    void SetNextInterpTm(const uint64& Time);
	double Interpolate(const uint64& TmMSecs) const;
	bool CanInterpolate(const uint64& Tm) const;

	static TStr GetType() { return "previous"; }
};

/////////////////////////////////////////
// Current point interpolator.
// Interpolate by returning the current point
class TCurrentPoint: public TBufferedInterpolator {
private:
	TCurrentPoint();
	TCurrentPoint(TSIn& SIn);

public:
	static PInterpolator New() { return new TCurrentPoint; }
	static PInterpolator New(TSIn& SIn) { return new TCurrentPoint(SIn); }

	void SetNextInterpTm(const uint64& Time);
	double Interpolate(const uint64& Tm) const;
	bool CanInterpolate(const uint64& Tm) const;

	static TStr GetType() { return "current"; }
};

/////////////////////////////////////////
// Linear interpolator.
// Interpolate by calculating point between two given points
class TLinear : public TBufferedInterpolator {
private:
	TLinear();
	TLinear(TSIn& SIn);
public:	
	static PInterpolator New() { return new TLinear; }
	static PInterpolator New(TSIn& SIn) { return new TLinear(SIn); }

	void SetNextInterpTm(const uint64& Tm);
	double Interpolate(const uint64& Tm) const;
	bool CanInterpolate(const uint64& Tm) const;

	static TStr GetType() { return "linear"; }
};

/////////////////////////////////////////
// Neural Networks - Neural Net
typedef enum { tanHyper, sigmoid, fastTanh, fastSigmoid, linear, softPlus } TTFunc;
//class TNNet {
ClassTP(TNNet, PNNet) //{
private:
    /////////////////////////////////////////
    // Neural Networks - Neuron
    class TLayer;    
    class TNeuron {
    private:
        static TRnd Rnd; //TODO: initialize it in the constructor with the 0

        TFlt RandomWeight(void) { return TFlt(Rnd.GetUniDev()); }
        TFlt TransferFcn(TFlt Sum);
        TFlt TransferFcnDeriv(TFlt Sum); // for back propagation learning
        // sum derivatives of weights in the next layer
        TFlt SumDOW(const TLayer& NextLayer) const;
        TFlt OutputVal;
        TFlt Gradient;
        TTFunc TFuncNm; // transfer function name
        TFltV SumDeltaWeight;
        // hash containing weight[0] and delta weight[1] for each outgoing edge
        TVec<TIntFltFltTr> OutEdgeV; 
        // Id of this neuron
        TInt Id;

    public:
        TNeuron();
        TNeuron(TInt OutputsN, TInt MyId, TTFunc TransFunc);
        TNeuron(TSIn& SIn);

        void SetOutVal(const TFlt& Val) { OutputVal = Val; }
        void SetDeltaWeight(const TInt& InNodeId, const TFlt& Val) { OutEdgeV[InNodeId].Val3 = Val; }
        void SetSumDeltaWeight(const TInt& InNodeId, const TFlt& Val) { SumDeltaWeight[InNodeId] = Val; }
        void SumDeltaWeights(const TInt& InNodeId, const TFlt& Val) { SumDeltaWeight[InNodeId] += Val; }
        void UpdateWeight(const TInt& InNodeId, const TFlt& Val) { OutEdgeV[InNodeId].Val2 += Val; }
        void UpdateInputWeights(TLayer& PrevLayer, const TFlt& LearnRate, const TFlt& Momentum, const TBool& UpdateWeights);

        TFlt GetOutVal(void) const { return OutputVal; }
        TFlt GetGradient(void) const { return Gradient; }
        TFlt GetDeltaWeight(const TInt& InNodeId) { return OutEdgeV[InNodeId].Val3; };
        TFlt GetWeight(const TInt& InNodeId) const { return OutEdgeV[InNodeId].Val2; };
        TFlt GetSumDeltaWeight(const TInt& InNodeId) const { return SumDeltaWeight[InNodeId]; };

        void FeedFwd(const TLayer& PrevLayer);
        void CalcOutGradient(TFlt TargVal);
        void CalcHiddenGradient(const TLayer& NextLayer);
        // Save the model
        void Save(TSOut& SOut);

		const TTFunc& GetFunction() { return TFuncNm; }
    };

    /////////////////////////////////////////
    // Neural Networks - Layer of neurons
    class TLayer {
    private:
        TVec<TNeuron> NeuronV;

    public:
        TLayer();
        TLayer(const TInt& NeuronsN, const TInt& OutputsN, const TTFunc& TransFunc);
        TLayer(TSIn& SIn);

        int GetNeuronN() const { return NeuronV.Len(); };
        TNeuron& GetNeuron(const TInt& NeuronN) { return NeuronV[NeuronN]; };
        TFlt GetOutVal(const TInt& NeuronN) const { return NeuronV[NeuronN].GetOutVal(); };
        TFlt GetGradient(const TInt& NeuronN) const { return NeuronV[NeuronN].GetGradient(); };
        TFlt GetWeight(const TInt& NeuronN, const TInt TargetId) const { return NeuronV[NeuronN].GetWeight(TargetId); };

        void SetOutVal(const TInt& NeuronN, const TFlt& Val) { NeuronV[NeuronN].SetOutVal(Val); };
        void UpdateInputWeights(const TInt& NeuronN, TLayer& PrevLayer,
        const TFlt& LearnRate, const TFlt& Momentum,
        const TBool& UpdateWeights) { NeuronV[NeuronN].UpdateInputWeights(PrevLayer, LearnRate, Momentum, UpdateWeights); };

        void CalcOutGradient(int& NeuronN, const TFlt& TargVal) { NeuronV[NeuronN].CalcOutGradient(TargVal); };
        void CalcHiddenGradient(int& NeuronN, const TLayer& NextLayer) { NeuronV[NeuronN].CalcHiddenGradient(NextLayer); };
        void FeedFwd(const TInt& NeuronN, const TLayer& PrevLayer) { NeuronV[NeuronN].FeedFwd(PrevLayer); };
        // Save the model
        void Save(TSOut& SOut);
    };

    TVec<TLayer> LayerV; 
    TFlt LearnRate; // [0.0..1.0] learning rate 
    TFlt Momentum; // [0.0..n] multiplier of last weight change
    TFlt Error;
    TFlt RecentAvgError;
    TFlt RecentAvgSmoothingFactor;


public:
    // constructor
    TNNet(const TIntV& LayoutV, const TFlt& _LearnRate = 0.1, 
            const TFlt& _Momentum = 0.5, const TTFunc& TFuncHiddenL = tanHyper,
            const TTFunc& TFuncOutL = tanHyper);
    TNNet(TSIn& SIn); // JOST: A rabim tudi nov PNNet za ta konstruktor?===
	static PNNet New(const TIntV& LayoutV, const TFlt& _LearnRate = 0.1, 
            const TFlt& _Momentum = 0.5, const TTFunc& TFuncHiddenL = tanHyper,
            const TTFunc& TFuncOutL = tanHyper)
			{ return new TNNet(LayoutV, _LearnRate, _Momentum, TFuncHiddenL, TFuncOutL); }
	static PNNet Load(TSIn& SIn);
    // Feed forward step
    void FeedFwd(const TFltV& InValV);
    // Back propagation step
    void BackProp(const TFltV& TargValV, const TBool& UpdateWeights = true);
    // TODO: try to make backprop in less for loops
    void GetResults(TFltV& ResultV) const;
    // Set learn rate
    void SetLearnRate(const TFlt& NewLearnRate) { LearnRate = NewLearnRate; };
	// set momentum
	void SetMomentum(const TFlt& NewMomentum) { Momentum = NewMomentum; }
    // Save the model
    void Save(TSOut& SOut) const;

	void GetLayout(TIntV& layout) {
		layout.Gen(LayerV.Len());
		for (int i = 0; i < LayerV.Len(); i++) {
			layout[i] = LayerV[i].GetNeuronN() - 1;
		}
	}
	TFlt GetLearnRate() { return LearnRate; }
	TFlt GetMomentum() { return Momentum; }
	TStr GetTFuncHidden() { 
		TStr FuncHidden = GetFunction(LayerV[1].GetNeuron(0).GetFunction());
		return FuncHidden;
	};
	TStr GetTFuncOut() {
		TStr FuncOut = GetFunction(LayerV[LayerV.Len() - 1].GetNeuron(0).GetFunction());
		return FuncOut;
	}
	TStr GetFunction(const TTFunc& Func);
};

/////////////////////////////////////////
// Recursive Linear Regression
ClassTP(TRecLinReg, PRecLinReg)// {
private:
	TFlt ForgetFact;	// forgetting factor
	TFlt RegFact;		// regularization

	TFullMatrix P;		// correlation matrix
	TVector Coeffs;		// model

	// copy constructor
	TRecLinReg(const TRecLinReg& LinReg);
	// move constructor
	TRecLinReg(const TRecLinReg&& LinReg);
	// load constructor
	TRecLinReg(TSIn& SIn);
	// default constructor
	TRecLinReg(const int& Dim, const double& _RegFact = 1.0, const double& _ForgetFact = 1.0);
public:	
	// new method
	static PRecLinReg New(const int& Dim, const double& RegFact, const double& ForgetFact)
			{ return new TRecLinReg(Dim, RegFact, ForgetFact); }
	static PRecLinReg Load(TSIn& SIn);

	virtual ~TRecLinReg() {}

	void Save(TSOut& SOut) const;	

	// assign operator
	TRecLinReg& operator =(TRecLinReg LinReg);

	// returns the dimension of the instances
	int GetDim() const { return Coeffs.Len(); }
	// returns the forgetting factor
	double GetForgetFact() const { return ForgetFact; }
	// returns the regulatization parameter
	double GetRegFact() const { return RegFact; }
	// learns a new sample
	void Learn(const TFltV& Sample, const double& SampleVal);
	// predicts a value
	double Predict(const TFltV& Sample);
	// copies the current models coefficients into the specified list
	void GetCoeffs(TFltV& Coef) const;
	// check if the coefficient vector contains NaN
	bool HasNaN() const;

	// set the forgetting factor
	void setForgetFact(const double& _ForgetFact) { ForgetFact = _ForgetFact; }
	// set the regularization
	void setRegFact(const double& _RegFact) { RegFact = _RegFact; }
	// set the dimensions
	void setDim(const int& _Dim) {
		P = TFullMatrix::Identity(_Dim) / RegFact;
		Coeffs = TVector(_Dim, true);
	}
};


/////////////////////////////////////////////////
/// Online histogram
///    Given a sequence of points b_1, ...,b_n
///    the class represents a frequency histogram for each interval [b_i, b_i+1)
///    The intervals are open on the right, except for the last interval [b_n-1 b_n]
///    The count data can be incremented or decremented, so we can work in an online
///    setting.
class TOnlineHistogram {
private:
	// state
	TFltV Counts; ///< Number of occurrences
	TFltV Bounds; ///< Interval bounds (Bounds.Len() == Counts.Len() + 1)
	TFlt Count; ///< Sum of counts
	// parameters
	TFlt MinCount; ///< If Count < MinCount, then IsInit returns false
public:	
	/// Constructs uninitialized object
	TOnlineHistogram() {};
	/// Constructs given bin parameters
	TOnlineHistogram(const double& LBound, const double& UBound, const int& Bins, const bool& AddNegInf, const bool& AddPosInf) { Init(LBound, UBound, Bins, AddNegInf, AddPosInf); }
	/// Constructs given JSON arguments
	TOnlineHistogram(const PJsonVal& ParamVal);
	/// Constructs from stream
	TOnlineHistogram(TSIn& SIn) : Counts(SIn), Bounds(SIn), Count(SIn) { }

	/// Initializes the object, resets current content is present
	void Init(const double& LBound, const double& UBound, const int& Bins, const bool& AddNegInf, const bool& AddPosInf);

	/// Resets the counts
	void Reset();

	/// Loads the model from stream
	void Load(TSIn& SIn) { *this = TOnlineHistogram(SIn); }
	/// Saves the model to stream
	void Save(TSOut& SOut) const { Counts.Save(SOut); Bounds.Save(SOut); SOut.Save(Count); }
	/// Finds the bin index given val, returns -1 if not found
	int FindBin(const double& Val) const;
	/// Increments the number of occurrences of values that fall within the same bin as Val
	void Increment(const double& Val);
	/// Decrements the number of occurrences of values that fall within the same bin as Val
	void Decrement(const double& Val);
	/// Returns the number of occurrences of values that fall within the same bin as Val
	double GetCount(const double& Val) const;
	/// Returns the number of bins
	int GetBins() const { return Counts.Len(); }
	/// Copies the count vector
	void GetCountV(TFltV& Vec) const { Vec = Counts; }
	/// Returns an element of count vector given index
	double GetCountN(const int& CountN) const { return Counts[CountN]; }
	/// Has the model beeen initialized and has sufficient data?
	bool IsInit() const { return Counts.Len() > 0 && Bounds.Len() > 0 && Count >= MinCount; }
	/// Clears the model
	void Clr() { Counts.Clr(); Bounds.Clr(); }
	/// Prints the model
	void Print() const;
	/// Returns a JSON representation of the model
	PJsonVal SaveJson() const;
};

/////////////////////////////////////////////////
///   TDigest
///   Data structure useful for percentile and quantile estimation for online data streams.
///   It can be added to any anomaly detector to set the number of alarms triggered as a percentage of the total samples.
///   This is the Data Lib Sketch Implementation: https://github.com/vega/datalib-sketch/blob/master/src/t-digest.js
///    Paper: Ted Dunning, Otmar Ertl - https://github.com/tdunning/t-digest/blob/master/docs/t-digest-paper/histo.pdf
class TTDigest {
private:
	TInt Nc;
	TInt Size;
	TInt Last;
	TFlt TotalSum;
	TFltV Weight;
	TFltV Mean;
	TFlt Min;
	TFlt Max;
	// double buffer to simplify merge operations
	// MergeWeight also used for transient storage of cumulative weights
	TFltV MergeWeight;
	TFltV MergeMean;

	// temporary buffers for recently added values
	TInt Tempsize;
	TFlt UnmergedSum;
	TInt TempLast;
	TFltV TempWeight;
	TFltV TempMean;
	TIntFltH Order; // for sorting

	TFlt EPSILON;
	TFlt DEFAULT_CENTROIDS;

	TFltV Quantiles;

	// Given the number of centroids, determine temp buffer size
	// Perform binary search to find value k such that N = k log2 k
	// This should give us good amortized asymptotic complexity
	TInt NumTemp(const TInt& N) const;
	// Converts a quantile into a centroid index value. The centroid index is
	// nominally the number k of the centroid that a quantile point q should
	// belong to. Due to round-offs, however, we can't align things perfectly
	// without splitting points and centroids. We don't want to do that, so we
	// have to allow for offsets.
	// In the end, the criterion is that any quantile range that spans a centroid
	// index range more than one should be split across more than one centroid if
	// possible. This won't be possible if the quantile range refers to a single
	// point or an already existing centroid.
	// We use the arcsin function to map from the quantile domain to the centroid
	// index range. This produces a mapping that is steep near q=0 or q=1 so each
	// centroid there will correspond to less q range. Near q=0.5, the mapping is
	// flatter so that centroids there will represent a larger chunk of quantiles.
	TFlt Integrate(TFlt Nc, TFlt Q_) const;

	TFlt MergeCentroid(TFlt Sum, TFlt& K1, TFlt& Wt, TFlt& Ut);

	void MergeValues();

	TInt Bisect(const TFltV& A, const TFlt& X, TInt Low, TInt& Hi) const;

	TFlt Boundary(const TInt& I, const TInt& J, const TFltV& U, const TFltV& W) const;

	TFlt Interp(const TFlt& X, const TFlt& X0, const TFlt& X1) const;

	void Init(const TInt& N);
public:
	/// Constructs uninitialized object
	TTDigest() {
		Init();
	}
    /// Constructs given JSON arguments
    TTDigest(const PJsonVal& ParamVal) {
		if (ParamVal->IsObjKey("clusters")) {
			Init(ParamVal->GetObjInt("clusters"));
		}
		else {
			Init();
		}
    };
	/// Constructs initialized object
	TTDigest(const TInt& N) {
		Init(N);
	};

	~TTDigest() {}

	/// Initializes the object, resets current content if present
	void Init();

	// Query for estimated quantile *q*.
	// Argument *q* is a desired quantile in the range (0,1)
	// For example, q = 0.5 queries for the median.
	TFlt GetQuantile(const TFlt& Q) const;

	TInt GetClusters() const;

	// Add a value to the t-digest.
	// Argument *v* is the value to add.
	// Argument *count* is the integer number of occurrences to add.
	// If not provided, *count* defaults to 1.
	void Update(const TFlt& V);

	void Update(const TFlt& V, const TFlt& Count);

	/// Prints the model
	void Print() const;
	/// Load from stream
	void LoadState(TSIn& SIn);
	/// Store state into stream
	void SaveState(TSOut& SOut) const;

};

/////////////////////////////////////////////////
///   TDigest
///   Ted Dunning, Otmar Ertl - https://github.com/tdunning/t-digest/blob/master/docs/t-digest-paper/histo.pdf
///   Gabriel Pichot C++ implementation: https://github.com/gpichot/cpp-tdigest
class TTDigestAvl {
private:
        TFlt Compression;
        TInt Count;
        TAvlTree* Centroids;
public: 

        /// Constructs uninitialized object
        TTDigestAvl(const TFltV& Quantiles) { Compression = 1000; Count = 0; Centroids = new TAvlTree(); };
        /// Constructs given JSON arguments
        TTDigestAvl(const PJsonVal& ParamVal) { Compression = 1000; Count = 0; Centroids = new TAvlTree(); };
        /// Constructs uninitialized object with compression
        TTDigestAvl (TFlt CompressionN): Compression(CompressionN) { Compression = CompressionN; Count = 0; Centroids = new TAvlTree();}
        /// Destructor
        ~TTDigestAvl() { delete Centroids;}

        /// Initializes the object, resets current content if present
        void Init();   

        // Number of digested inputs
        TInt Size() { return Count; }
        void Compress();
        void Update(TFlt X, TFlt W) {
            int Start = Centroids->Floor(X);
            if(Start == 0) {
                Start = Centroids->First();
            }

            if(Start == 0) {
                EAssert(Centroids->GetSize() == 0);
                Centroids->Add(X, (int)W);
                Count += W;
            } else {
                double MinDistance = DBL_MAX;
                int LastNeighbor = 0;
                for(TInt Neighbor = Start; Start != 0; Neighbor = Centroids->NextNode(Neighbor)) {
                    double Z = TFlt::Abs(Centroids->GetValue(Neighbor) - X);
                    if(Z < MinDistance) {
                        Start = Neighbor;
                        MinDistance = Z;
                    } else {
                        LastNeighbor = Neighbor;
                        break;
                    }
                }

                TFlt Closest = 0;
                TInt Sum = Centroids->CeilSum(Start);
                TFlt N = 0;
                for(TInt Neighbor = Start; Neighbor != LastNeighbor; Neighbor = Centroids->NextNode(Neighbor)) {
                    TFlt Q = 0.5;
                    if (Count != 1) {
                    	Q = (Sum + (Centroids->GetCount(Neighbor) - 1 / 2. )) / (Count - 10);
                    }

                    const TFlt K = 4.0 * Count * Q * (1.0 - Q) / Compression;
                    //const TFlt K = TMath::Sqrt(Count * Q * (1.0 - Q));
                    //const TFlt K = sqrt(Count * Q * (1.0 - Q));

                    if(Centroids->GetCount(Neighbor) + W <= K) {
                        N++;
                        TFlt R = (float)rand() / RAND_MAX;

                        if(R < 1.0 / N) {
                            Closest = Neighbor;
                        }
                    }
                    Sum += Centroids->GetCount(Neighbor);
                }

                if(Closest == 0.0) {
                    Centroids->Add(X, (int)W);
                } else {
                    Centroids->Update((int)Closest, X, (int)W);
                }
                Count += W;

                if(Centroids->GetSize() > 100 * Compression) {
                    Compress();
                }
            }
        }
        void Update(TFlt x) { Update(x, 1.0); }
        TAvlTree* GetCentroids() const {
            return Centroids;
        }
        TInt CentroidsCount() const { return Centroids->GetSize();}
        TFlt Quantile(const TFlt& PreviousIndex, const TFlt& Index, const TFlt& NextIndex, const TFlt& PreviousMean, const TFlt& NextMean) const {
        	const TFlt Delta = NextIndex - PreviousIndex;
            const TFlt PreviousWeight = (NextIndex - Index) / Delta;
            const TFlt NextWeight = (Index - PreviousIndex) / Delta;
            return PreviousMean * PreviousWeight + NextMean * NextWeight;
        }
        TFlt Quantile(const TFlt& Q) const;
        void Merge(TTDigestAvl* Digest) {
                    TAvlTree* CentroidsN = Digest->GetCentroids();
                    for(TInt N = CentroidsN->First(); N != 0; N = CentroidsN->NextNode(N)) {
                    	TFlt A = CentroidsN->GetValue(N);
                    	TInt B = CentroidsN->GetCount(N);
                        Update(A, (double)B);
                    }
                }
        TFlt GetQuantile(const TFlt& Q) const { return Quantile(Q); }
        /// Prints the model
        void Print() const;
        /// Load from stream
        void LoadState(TSIn& SIn);
        /// Store state into stream
        void SaveState(TSOut& SOut) const;
};

/////////////////////////////////////////////////
/// Chi square
class TChiSquare {
private:	     
	// state
	TFlt Chi2;
	TFlt P;
	// parameters
	TInt DegreesOfFreedom;
public:
	TChiSquare() : P(TFlt::PInf) { }
	TChiSquare(const PJsonVal& ParamVal);
	/// Reset
	void Reset() { Chi2 = 0; P = TFlt::PInf; }
	/// Compute two sample chi2 test
	void Update(const TFltV& OutValVX, const TFltV& OutValVY);
	/// Return Chi2 value
	double GetChi2() const { return Chi2; }
	/// Return P value
	double GetP() const { return P; }
	int GetDof() const {return DegreesOfFreedom;}
	/// Prints the model
	void Print() const;

	/// Load from stream
	void LoadState(TSIn& SIn);
	/// Store state into stream
	void SaveState(TSOut& SOut) const;
};

/////////////////////////////////////////////////
///   Count-Min Sketch
///   Muthukrishnan, Cormode (2004) - http://dimacs.rutgers.edu/~graham/pubs/papers/cmsoft.pdf
///   Daniel Alabi C++ implementation: https://github.com/alabid/countminsketch
class TCountMinSketch {
private:
        TInt W;
        TInt D;
        TFlt Eps; // Error rate 0.01 < Eps < 1
        TFlt Gamma; // Probability for accuracy, 0 < Gamma < 1
        TInt aj, bj;
        TIntV InVals;
        // array of arrays of counters
        int **C;
        // array of hash values for a particular item
        // contains two element arrays {aj,bj}
        int **Hashes;
        // generate "new" aj,bj
        void Genajbj(int **hashes, int i);
public:
        TInt Total;
        TIntV Counts;
        TCountMinSketch(const PJsonVal& ParamVal);
        TCountMinSketch(TFlt eps, TFlt gamma);
        // update item (int) by count c
        void Update(TInt Item, TInt C);
        // estimate count of item i and return count
        TInt Estimate(TInt item) const;
        // return total count
        TInt GetTotalCount() const;
        /// Load from stream
        void LoadState(TSIn& SIn);
        /// Store state into stream
        void SaveState(TSOut& SOut) const;
        void Print() const;
};

}

#endif
